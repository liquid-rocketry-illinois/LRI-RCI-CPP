#ifndef VIRTUAL_H
#define VIRTUAL_H
#include <string>

#include "RCP_Interface.h"

namespace LRI::RCI {
    class VirtualPort : public RCP_Interface {
    public:
        explicit VirtualPort() = default;
        ~VirtualPort() override = default;
        bool isOpen() const override;
        bool pktAvailable() const override;
        size_t sendData(const void* data, size_t length) const override;
        size_t readData(void* data, size_t bufferSize) const override;
        std::string interfaceType() const override;
    };
}

#endif //VIRTUAL_H
