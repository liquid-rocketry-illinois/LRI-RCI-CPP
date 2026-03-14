#include "interfaces/VirtualPort.h"

#include <iostream>

namespace LRI::RCI {
    VirtualPort::VirtualPort() : initpacket(7) {
        // Push the "target ready" packet so the UI can be interacted with
        initpacket.push(0x05);
        initpacket.push(RCP_DEVCLASS_TEST_STATE);
        initpacket.push(0x00);
        initpacket.push(0x00);
        initpacket.push(0x00);
        initpacket.push(0x00);
        initpacket.push(0x10 | RCP_TEST_STOPPED);
    }

    // Everything here is basically just stubs
    bool VirtualPort::isOpen() const { return true; }

    bool VirtualPort::pktAvailable() const {
        return initpacket.size() >= static_cast<uint32_t>(initpacket.peek() & (~RCP_CHANNEL_MASK)) + 2;
    }

    size_t VirtualPort::sendData([[maybe_unused]] const void* data, size_t length) const {
        const auto* idata = static_cast<const uint8_t*>(data);
        std::cout << "Sent: " << std::hex;
        for(size_t i = 0; i < length; i++) { std::cout << static_cast<int>(idata[i]) << "  "; }
        std::cout << std::endl;
        return length;
    }

    size_t VirtualPort::readData(void* data, size_t bufferSize) const {
        size_t written = 0;
        uint8_t* cdata = static_cast<uint8_t*>(data);
        for(size_t i = 0; i < bufferSize && !initpacket.isEmpty(); written++, i++) {
            cdata[i] = initpacket.pop();
        }

        return written;
    }

    std::string VirtualPort::interfaceType() const { return "Virtual Interface"; }
} // namespace LRI::RCI
