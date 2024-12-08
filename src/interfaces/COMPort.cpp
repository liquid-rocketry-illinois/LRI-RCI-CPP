#include "interfaces/COMPort.h"
#include <iostream>
#include <sstream>
#include <RCP_Host/RCP_Host.h>

namespace LRI::RCI {
    COMPort::COMPort(const char* _portname, DWORD baudrate) {
        portname = new char[strlen(_portname)];
        strcpy(portname, _portname);
        port = CreateFile(portname, GENERIC_READ | GENERIC_WRITE, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);

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

        buffer = new RingBuffer(bufferSize);
        read = true;
        thread = new std::thread(&COMPort::threadRead, this);
    }

    COMPort::~COMPort() {
        close();
    }

    bool COMPort::close() {
        open = false;
        read = false;
        thread->join();
        delete thread;
        thread = nullptr;
        delete buffer;
        buffer = nullptr;
        return !CloseHandle(port);
    }

    bool COMPort::isOpen() const {
        return open;
    }

    DWORD COMPort::lastError() const {
        return lastErrorVal;
    }

    size_t COMPort::sendData(const void* bytes, size_t length) const {
        DWORD bytes_written = 0;

        dataAccess.lock();
        if(!WriteFile(port, bytes, length, &bytes_written, nullptr)) return -1;
        dataAccess.unlock();
        return bytes_written;
    }

    size_t COMPort::readData(void* rawbytes, size_t bufferlength) const {
        int bytesread;
        uint8_t* bytes = (uint8_t*) rawbytes;

        dataAccess.lock();
        for(bytesread = 0; buffer->size() > 0 && bytesread < bufferlength; bytesread++) {
            bytes[bytesread] = buffer->pop();
        }
        dataAccess.unlock();

        return bytesread;
    }

    std::string COMPort::interfaceType() const {
        return std::string("Serial Port (") + portname + ")";
    }

    bool COMPort::pktAvailable() const {
        dataAccess.lock();
        bool hasData;
        if(buffer->size() == 0) hasData = false;
        else {
            hasData = buffer->size() > (buffer->peek() & (~RCP_CHANNEL_MASK));
        }

        dataAccess.unlock();
        return hasData;
    }

    void COMPort::threadRead() {
        while(read) {
            dataAccess.lock();
            if(buffer->size() >= bufferSize) {
                dataAccess.unlock();
                continue;
            }
            dataAccess.unlock();
            uint8_t byte;
            DWORD read;

            if(!ReadFile(port, &byte, 1, &read, nullptr) || read != 1) {
                lastErrorVal = GetLastError();
                continue;
            }

            dataAccess.lock();
            buffer->push(byte);
            dataAccess.unlock();
        }
    }

    COMPort::RingBuffer::RingBuffer(int _buffersize) :
        buffersize(_buffersize), datastart(0), dataend(0), data(nullptr) {
        data = new uint8_t[buffersize];
        memset(data, 0, buffersize);
    }

    COMPort::RingBuffer::~RingBuffer() {
        delete[] data;
    }

    int COMPort::RingBuffer::size() {
        return dataend - datastart;
    }

    uint8_t COMPort::RingBuffer::pop() {
        if(size() == 0) return 0;
        uint8_t retval = data[datastart % buffersize];
        datastart++;
        return retval;
    }

    uint8_t COMPort::RingBuffer::peek() {
        return data[datastart % buffersize];
    }


    void COMPort::RingBuffer::push(uint8_t value) {
        data[dataend] = value;
        dataend++;
    }
}
