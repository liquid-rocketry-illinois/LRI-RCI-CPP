#include "devices/COMPort.h"
#include <iostream>

namespace LRI::RCI {
    COMPort::COMPort(const char* portname, DWORD baudrate) {
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

        if(!SetCommState(port, &params)) {
            lastErrorVal = GetLastError();
            return;
        }

        open = true;
        lastErrorVal = 0;
    }

    COMPort::~COMPort() {
        close();
    }

    bool COMPort::close() {
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

        if(!WriteFile(port, bytes, length, &bytes_written, nullptr)) return -1;
        return bytes_written;
    }

    size_t COMPort::readData(void* bytes, size_t bufferlength) const {
        DWORD bytes_read = 0;

        if(!ReadFile(port, bytes, bufferlength, &bytes_read, nullptr)) return -1;
        return bytes_read;
    }
}
