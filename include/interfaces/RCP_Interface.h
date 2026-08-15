#ifndef RCP_INTERFACE_H
#define RCP_INTERFACE_H

#include <string>

namespace LRI::RCI {
    // An interface representing an interface RCP can use to communicate. There are a few function implementations
    // embedded in the class declaration, just because they are so short and it would be so awful to make a whole file
    // for such a trivial constructor and getter
    class RCP_Interface {
    protected:
        const std::string displayName;

    public:
        explicit RCP_Interface(std::string displayName) : displayName(std::move(displayName)) {}

        // Send and receive functions
        virtual size_t sendData(const void* data, size_t length) const = 0;
        virtual size_t readData(void* data, size_t bufferSize) const = 0;

        // Returns true if interface is open
        [[nodiscard]] virtual bool isOpen() const = 0;

        // Returns true if a full packet is ready to be read from the interface
        [[nodiscard]] virtual bool pktAvailable() const = 0;

        [[nodiscard]] virtual size_t bytesWaiting() const { return 0; }

        // Returns a display string representing the interface
        [[nodiscard]] const std::string& interfaceType() const { return displayName; }

        virtual ~RCP_Interface() = default;
    };
} // namespace LRI::RCI

#endif // RCP_INTERFACE_H
