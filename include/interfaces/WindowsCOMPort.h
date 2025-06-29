#ifndef WINDOWSCOMPORT_H
#define WINDOWSCOMPORT_H
#ifdef _WIN32

#include <Windows.h>
#include "interfaces/COMPort.h"

namespace LRI::RCI {
    class COMPort : public ICOMPort {
    protected:
        virtual void threadRead() override;

    public:
        COMPort(std::string portName, unsinged long baudRate);
    };
} // namespace LRI::RCI

#endif // _WIN32
#endif // WINDOWSCOMPORT_H
