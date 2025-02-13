#include "interfaces/COMPort.h"
#include <RCP_Host/RCP_Host.h>
#include <iostream>

namespace LRI::RCI {
    // Quite an awful looking constructor if I do say so myself
    COMPort::COMPort(const char* _portname, const DWORD& _baudrate)
            : portname(new char[strlen(_portname)]), baudrate(_baudrate),
              port(CreateFile(
                      _portname, GENERIC_READ | GENERIC_WRITE, 0, nullptr,
                      OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr)),
              lastErrorVal(0), inbuffer(nullptr), outbuffer(nullptr), thread(nullptr), doComm(false) {
        strcpy(portname, _portname); // Windows says this is deprecated because it likes to be special

        // If the port is invalid exit constructor
        if(port == INVALID_HANDLE_VALUE) {
            lastErrorVal = GetLastError();
            return;
        }

        // Setup serial parameters, such as parity, flow control, and baud rate
        DCB params = {0};
        params.DCBlength = sizeof(DCB);

        if(!GetCommState(port, &params)) {
            lastErrorVal = GetLastError();
            return;
        }

        params.BaudRate = baudrate;
        params.ByteSize = 8;
        params.StopBits = ONESTOPBIT;
        params.Parity = NOPARITY;
        params.fDtrControl = DTR_CONTROL_ENABLE;

        if(!SetCommState(port, &params)) {
            lastErrorVal = GetLastError();
            return;
        }

        // Timeouts for IO operations
        COMMTIMEOUTS timeouts = {0};
        timeouts.ReadIntervalTimeout = 50;
        timeouts.ReadTotalTimeoutConstant = 50;
        timeouts.ReadTotalTimeoutMultiplier = 10;
        timeouts.WriteTotalTimeoutConstant = 50;
        timeouts.WriteTotalTimeoutMultiplier = 10;

        if(!SetCommTimeouts(port, &timeouts)) {
            lastErrorVal = GetLastError();
            return;
        }

        // If we get to here then the connection has been successfully created
        open = true;
        lastErrorVal = 0;

        // Create in and out buffers
        inbuffer = new RCI::RingBuffer<uint8_t>(bufferSize);
        outbuffer = new RCI::RingBuffer<uint8_t>(bufferSize);

        // Start IO thread
        doComm = true;
        thread = new std::thread(&COMPort::threadRead, this);
    }

    // Destructor just needs to close
    COMPort::~COMPort() {
        close();
    }

    // Closes and cleans up port
    bool COMPort::close() {
        // Stop IO thread
        doComm = false;

        // Mark as no longer open
        open = false;
        ready = false;

        // Join and delete IO thread
        if(thread) thread->join();
        delete thread;
        thread = nullptr;

        // Delete buffers
        delete inbuffer;
        inbuffer = nullptr;
        delete outbuffer;
        outbuffer = nullptr;

        // Close the windows handle
        return !CloseHandle(port);
    }

    bool COMPort::isOpen() const {
        return open;
    }

    bool COMPort::isReady() const {
        return ready;
    }

    DWORD COMPort::lastError() const {
        return lastErrorVal;
    }

    // SendData doesn't actually write to the handle. It writes to the intermediary buffers and the IO thread does the
    // writing
    size_t COMPort::sendData(const void* bytes, const size_t length) const {
        // Lock the output buffer, and check if there is space to insert the data. If not, return
        outlock.lock();
        if(outbuffer->size() + length > outbuffer->capacity()) {
            outlock.unlock();
            return 0;
        }

        // Push new bytes to the buffer
        const auto _bytes = static_cast<const uint8_t*>(bytes);
        for(size_t i = 0; i < length; i++) {
            outbuffer->push(_bytes[i]);
        }

        // Return
        outlock.unlock();
        return length;
    }

    // Reading is similar to writing. IO thread does the work, this just reads from the buffer
    size_t COMPort::readData(void* bytes, size_t bufferlength) const {
        int bytesread;
        const auto _bytes = static_cast<uint8_t*>(bytes);

        // Lock the input buffer and pop bytes from the buffer and place them into the output buffer
        inlock.lock();
        for(bytesread = 0; inbuffer->size() > 0 && bytesread < bufferlength; bytesread++) {
            _bytes[bytesread] = inbuffer->pop();
        }

        inlock.unlock();
        return bytesread;
    }

    // Returns the display string
    std::string COMPort::interfaceType() const {
        return std::string("Serial Port (") + portname + " @ " + std::to_string(baudrate) + " baud)";
    }

    // Determines if a full packet is available by treating the front byte in the input buffer as an RCP header byte.
    // Partial RCP packets should never be read
    bool COMPort::pktAvailable() const {
        inlock.lock();
        bool hasData;
        if(inbuffer->size() == 0) hasData = false;
        else hasData = inbuffer->size() >= (inbuffer->peek() & (~RCP_CHANNEL_MASK)) + 2;
        inlock.unlock();
        return hasData;
    }

    // The actual work. Alternates between a read and a write on each loop
    void COMPort::threadRead() {
        PurgeComm(port, PURGE_RXABORT | PURGE_RXCLEAR | PURGE_TXABORT | PURGE_TXCLEAR);
        using namespace std::chrono_literals;
        std::this_thread::sleep_for(3000ms);
        inlock.lock();
        inbuffer->clear();
        inlock.unlock();

        outlock.lock();
        outbuffer->clear();
        outlock.unlock();

        ready = true;

        bool r_nw = false;
        while(doComm) {
            if(r_nw) {
                // Read cycle
                r_nw = false;

                // If there is no space in the buffer return
                inlock.lock();
                bool res = inbuffer->size() >= inbuffer->capacity();
                inlock.unlock();
                if(res) continue;

                uint8_t byte;
                DWORD read;

                // Read the latest byte from the serial port and write it to inbuffer
                if(!ReadFile(port, &byte, 1, &read, nullptr)) {
                    lastErrorVal = GetLastError();
                    continue;
                }

                if(read == 0) continue;

                inlock.lock();
                inbuffer->push(byte);
                inlock.unlock();
//                std::cout << "rcv: " << std::hex << (int) byte << std::endl;
            }

            else {
                // Write cycle
                r_nw = true;

                // If the output buffer is empty return
                outlock.lock();
                bool res = outbuffer->size() == 0;
                if(res) {
                    outlock.unlock();
                    continue;
                }

                // The byte is peeked so that if the write fails the byte remains in the buffer
                uint8_t byte = outbuffer->peek();
                outlock.unlock();

                DWORD written;

                // Write to serial device
                if(!WriteFile(port, &byte, 1, &written, nullptr) || written != 1) {
                    lastErrorVal = GetLastError();
                    continue;
                }

                // Pop byte from buffer if successful
                outlock.lock();
                outbuffer->pop();
                outlock.unlock();
//                std::cout << "snd: " << std::hex << (int) byte << std::endl;
            }
        }
    }
}
