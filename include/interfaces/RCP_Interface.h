#ifndef RCP_INTERFACE_H
#define RCP_INTERFACE_H

namespace LRI::RCI {
    class RCP_Interface {
    public:
        virtual size_t sendData(const void* data, size_t length) const = 0;
        virtual size_t readData(void* data, size_t bufferSize) const = 0;
        virtual bool isOpen() const = 0;
        virtual bool hasData() const = 0;
        virtual std::string interfaceType() const = 0;
        virtual ~RCP_Interface() = default;
    };
}

#endif //RCP_INTERFACE_H
