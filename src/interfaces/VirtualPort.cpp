#include "interfaces/VirtualPort.h"

namespace LRI::RCI {
    bool VirtualPort::isOpen() const {
        return true;
    }

    bool VirtualPort::hasData() const {
        return false;
    }

    size_t VirtualPort::sendData(const void* data, size_t length) const {
        return length;
    }

    size_t VirtualPort::readData(void* data, size_t bufferSize) const {
        memset(data, 0, bufferSize);
        return bufferSize;
    }

    std::string VirtualPort::interfaceType() const {
        return "Virtual Interface";
    }
}