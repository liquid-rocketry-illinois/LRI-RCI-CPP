#include "interfaces/COMPort.h"

namespace LRI::RCI {
    COMPort::COMPort(const std::string&& portname, unsigned long baudrate) :
        portname(portname), baudrate(baudrate), port(nullptr) {
        ioUnlock();
    }

    COMPort::~COMPort() { ioLock(); }

    std::string COMPort::interfaceType() const {
        return std::string("Serial Port (") + portname + " @ " + std::to_string(baudrate) + " baud)";
    }

    void COMPort::ioInit() {}

    bool COMPort::writeBytes(const uint8_t* bytes, size_t length) { return false; }

    bool COMPort::readBytes(uint8_t* bytes, size_t bufLength, size_t& written) { return false; }

    void COMPort::ioDeinit() {}

    bool COMPort::enumSerialDevs(std::vector<std::pair<std::string, std::string>>& portlist) { return false; }
} // namespace LRI::RCI
