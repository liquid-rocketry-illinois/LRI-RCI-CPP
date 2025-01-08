#include "interfaces/COMPort.h"
#include <iostream>
#include <sstream>
#include <RCP_Host/RCP_Host.h>

namespace LRI::RCI {
    COMPort::COMPort(const char* _portname, const DWORD& _baudrate)
        : portname(new char[strlen(_portname)]), baudrate(_baudrate),
          port(CreateFile(
              _portname, GENERIC_READ | GENERIC_WRITE, 0, nullptr,
              OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr)),
          lastErrorVal(0), inbuffer(nullptr), outbuffer(nullptr), thread(nullptr), doComm(false) {
        strcpy(portname, _portname);

        if(port == INVALID_HANDLE_VALUE) {
            lastErrorVal = GetLastError();
            return;
        }

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

        open = true;
        lastErrorVal = 0;

        inbuffer = new RCI::RingBuffer<uint8_t>(bufferSize);
        outbuffer = new RCI::RingBuffer<uint8_t>(bufferSize);

        doComm = true;
        thread = new std::thread(&COMPort::threadRead, this);
    }

    COMPort::~COMPort() {
        close();
    }

    bool COMPort::close() {
        doComm = false;
        open = false;
        ready = false;
        if(thread) thread->join();
        delete thread;
        thread = nullptr;

        delete inbuffer;
        inbuffer = nullptr;
        delete outbuffer;
        outbuffer = nullptr;

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

    size_t COMPort::sendData(const void* bytes, const size_t length) const {
        outlock.lock();
        if(outbuffer->size() + length > outbuffer->capacity()) {
            outlock.unlock();
            return 0;
        }

        const auto _bytes = static_cast<const uint8_t*>(bytes);
        for(size_t i = 0; i < length; i++) {
            outbuffer->push(_bytes[i]);
        }

        outlock.unlock();
        return length;
    }

    size_t COMPort::readData(void* bytes, size_t bufferlength) const {
        int bytesread;
        const auto _bytes = static_cast<uint8_t*>(bytes);

        inlock.lock();
        for(bytesread = 0; inbuffer->size() > 0 && bytesread < bufferlength; bytesread++) {
            _bytes[bytesread] = inbuffer->pop();
        }

        inlock.unlock();
        return bytesread;
    }

    std::string COMPort::interfaceType() const {
        return std::string("Serial Port (") + portname + " @ " + std::to_string(baudrate) + " baud)";
    }

    bool COMPort::pktAvailable() const {
        inlock.lock();
        bool hasData;
        if(inbuffer->size() == 0) hasData = false;
        else hasData = inbuffer->size() >= (inbuffer->peek() & (~RCP_CHANNEL_MASK)) + 2;
        inlock.unlock();
        return hasData;
    }

    void COMPort::threadRead() {
        using namespace std::chrono_literals;
        std::this_thread::sleep_for(5000ms);
        ready = true;

        bool r_nw = false;
        while(doComm) {
            if(r_nw) {
                r_nw = false;

                inlock.lock();
                bool res = inbuffer->size() >= inbuffer->capacity();
                inlock.unlock();
                if(res) continue;

                uint8_t byte;
                DWORD read;

                if(!ReadFile(port, &byte, 1, &read, nullptr)) {
                    lastErrorVal = GetLastError();
                    continue;
                }

                if(read == 0) continue;

                inlock.lock();
                inbuffer->push(byte);
                inlock.unlock();
                std::cout << "Read byte: " << std::hex << (int) byte << std::endl;
            }

            else {
                r_nw = true;

                outlock.lock();
                bool res = outbuffer->size() == 0;
                if(res) {
                    outlock.unlock();
                    continue;
                }

                uint8_t byte = outbuffer->peek();
                outlock.unlock();

                DWORD written;

                if(!WriteFile(port, &byte, 1, &written, nullptr) || written != 1) {
                    lastErrorVal = GetLastError();
                    continue;
                }

                outlock.lock();
                outbuffer->pop();
                outlock.unlock();
            }
        }
    }
}
