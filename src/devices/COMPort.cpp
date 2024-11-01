#include "devices/COMPort.h"

namespace LRI {
    COMPort::COMPort(const char* portname) {
        port = CreateFile(portname, GENERIC_READ | GENERIC_WRITE, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);

        if(port == INVALID_HANDLE_VALUE) {
            if(GetLastError() == ERROR_FILE_NOT_FOUND) {
                std::cout << "COM Port not open" << std::endl;
            }

            else {
                std::cout << "Some other error occured: " << GetLastError() << std::endl;
            }

            return;
        }

        DCB params = {0};
        params.DCBlength = sizeof(DCB);

        if(!GetCommState(port, &params)) {
            std::cout << "Error getting COM state: " << GetLastError() << std::endl;
            return;
        }

        params.BaudRate = CBR_115200;
        params.ByteSize = 8;
        params.StopBits = ONESTOPBIT;
        params.Parity = NOPARITY;

        if(!SetCommState(port, &params)) {
            std::cout << "Error setting COM state: " << GetLastError() << std::endl;
            return;
        }

        open = true;
    }

    bool COMPort::isOpen() {
        return open;
    }

    DWORD COMPort::write(uint8_t* bytes, int towrite) {
        DWORD bytes_written = 0;

        if(!WriteFile(port, bytes, towrite, &bytes_written, nullptr)) return -1;
        return bytes_written;
    }

    DWORD COMPort::read(uint8_t* bytes, int toread) {
        DWORD bytes_read = 0;

        if(!ReadFile(port, bytes, toread, &bytes_read, nullptr)) return -1;
        return bytes_read;
    }

    bool COMPort::close() {
        return !CloseHandle(port);
    }

    COMPort::~COMPort() {
        close();
    }

}