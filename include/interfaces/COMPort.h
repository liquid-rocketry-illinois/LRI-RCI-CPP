#ifndef COMPORT_H
#define COMPORT_H

#ifdef _WIN32
#include <Windows.h>
#endif

#include <string>

#include "IOInterface.h"
#include "UI/TargetChooser.h"

namespace LRI::RCI {
    class COMPort : public IOInterface {
        static constexpr int bufferSize = 1'048'576;

#ifdef _WIN32
        using NativeHandle = HANDLE;
#else
        using NativeHandle = void*;
#endif

        const std::string portname;
        const unsigned long baudrate;

        NativeHandle port;

    public:
        // COMPort needs the port name as a string (COMx) and the baud rate
        explicit COMPort(const std::string&& portname, unsigned long baudrate);
        ~COMPort() override;

        // A display string representing the port (COMx @ y baud)
        std::string interfaceType() const override;

        void ioInit() override;
        bool writeBytes(const uint8_t* bytes, size_t length) override;
        bool readBytes(uint8_t* bytes, size_t bufLength, size_t& written) override;
        void ioDeinit() override;

        // Helper to enumerate detected serial devices
        static bool enumSerialDevs(std::vector<std::pair<std::string, std::string>>& portlist);
    };
} // namespace LRI::RCI
#endif // COMPORT_H
