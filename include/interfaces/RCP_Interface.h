#ifndef RCP_INTERFACE_H
#define RCP_INTERFACE_H

#include <string>

namespace LRI::RCI {
    // An interface representing an interface RCP can use to communicate
    class RCP_Interface {
    public:
        // Send and receive functions
        virtual size_t sendData(const void* data, size_t length) const = 0;
        virtual size_t readData(void* data, size_t bufferSize) const = 0;

        // Returns true if interface is open
        [[nodiscard]] virtual bool isOpen() const = 0;

        // Returns true if a full packet is ready to be read from the interface
        [[nodiscard]] virtual bool pktAvailable() const = 0;

        // Returns a display string representing the interface
        [[nodiscard]] virtual std::string interfaceType() const = 0;

        virtual ~RCP_Interface() = default;
    };
}

#endif //RCP_INTERFACE_H
