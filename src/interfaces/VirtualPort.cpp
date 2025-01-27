#include "interfaces/VirtualPort.h"

namespace LRI::RCI {
    // Everything here is basically just stubs
    bool VirtualPort::isOpen() const {
        return true;
    }

    bool VirtualPort::pktAvailable() const {
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