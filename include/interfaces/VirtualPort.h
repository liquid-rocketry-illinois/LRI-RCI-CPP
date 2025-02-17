#ifndef VIRTUAL_H
#define VIRTUAL_H

#include <string>

#include "RCP_Interface.h"
#include "UI/TargetChooser.h"

namespace LRI::RCI {

    // A testing interface. The send and receive functions are stubs and do not function
    class VirtualPort : public RCP_Interface {
    public:
        explicit VirtualPort() = default;
        ~VirtualPort() override = default;

        // isOpen always returns true, and pktAvailable always returns false
        [[nodiscard]] bool isOpen() const override;
        [[nodiscard]] bool pktAvailable() const override;

        // Send function does nothing except return the inputted length
        size_t sendData(const void* data, size_t length) const override;

        // Read function does nothing but zero the inputted buffer and return bufferSize
        size_t readData(void* data, size_t bufferSize) const override;

        // Returns "Virtual Interface"
        [[nodiscard]] std::string interfaceType() const override;
    };

    // A testing interface for debugging. See interfaces/VirtualPort.h
    class VirtualPortChooser final : public InterfaceChooser {
    public:
        VirtualPortChooser() = default;
        RCP_Interface* render() override;
    };
}

#endif //VIRTUAL_H
