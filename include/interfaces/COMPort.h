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
        const bool arduinoMode;

        NativeHandle port;

    public:
        // COMPort needs the port name as a string (COMx) and the baud rate
        explicit COMPort(const std::string&& portname, unsigned long baudrate, bool arduinoMode);
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

    // This chooser is for connecting to serial devices (e.g. COM1, COM2, so on). It allows selecting a device
    // and choosing the baud rate
    class COMPortChooser final : public InterfaceChooser {
        // Storage for available ports. Ports will be in the format of their handle name, a colon, and the windows
        // display name (ex. COM1:Arduino Serial Device)
        std::vector<std::pair<std::string, std::string>> portlist;

        // The index of the selected port
        size_t selectedPort;

        // If there was an error
        bool error;

        // The current baud rate
        int baud;

        bool arduinoMode;

        // The interface itself
        COMPort* port;

    public:
        explicit COMPortChooser();

        // Renders the UI for the chooser, and returns a pointer to a valid and open interface once an interface
        // has been successfully created
        RCP_Interface* render() override;
    };
} // namespace LRI::RCI
#endif // COMPORT_H
