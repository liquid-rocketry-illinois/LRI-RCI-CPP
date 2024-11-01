#ifndef COMPORT_H
#define COMPORT_H

#include <Windows.h>
#include <iostream>

namespace LRI {
    class COMPort {
        HANDLE port;
        bool open = false;

    public:
        COMPort(const char* portname);
        ~COMPort();
        DWORD write(uint8_t* bytes, int towrite);
        DWORD read(uint8_t* bytes, int toread);
        bool close();
        bool isOpen();
    };
}
#endif //COMPORT_H
