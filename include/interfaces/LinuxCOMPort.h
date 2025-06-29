#ifndef LINUXCOMPORT_H
#define LINUXCOMPORT_H
#ifndef _WIN32

#include "ICOMPort.h"

namespace LRI::RCI {
    class COMPort : public ICOMPort {
    protected:
        void threadRead() override;

    public:
        COMPort(std::string portName, unsigned long baudRate);
    };
}

#endif // _WIN32
#endif //LINUXCOMPORT_H
