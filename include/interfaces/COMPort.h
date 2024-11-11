#ifndef COMPORT_H
#define COMPORT_H

#include <string>
#include <Windows.h>
#include "RCP_Interface.h"

namespace LRI::RCI {
    class COMPort : public RCP_Interface {
        char* portname;
        HANDLE port;
        bool open = false;
        DWORD lastErrorVal;

    public:
        explicit COMPort(const char* portname, DWORD baudrate);
        ~COMPort();
        bool close();
        bool isOpen() const override;
        bool hasData() const override;
        DWORD lastError() const;
        size_t sendData(const void* bytes, size_t length) const override;
        size_t readData(void* bytes, size_t buffersize) const override;
        std::string interfaceType() const override;
    };
}
#endif //COMPORT_H
