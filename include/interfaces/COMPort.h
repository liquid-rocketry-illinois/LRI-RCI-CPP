#ifndef COMPORT_H
#define COMPORT_H

#include <vector>
#include <atomic>
#include <mutex>
#include <thread>

#include "RCP_Interface.h"
#include "utils.h"

namespace LRI::RCI {
    class ICOMPort : public RCP_Interface {
    protected:
        static constexpr int BUFFER_SIZE = 1'048'576;

        const std::string portName;
        const unsigned long baudRate;

        std::atomic_bool open;
        std::atomic_bool ready;

        std::atomic_ulong lastErrorVal;

        RingBuffer<uint8_t>* const inbuffer;
        RingBuffer<uint8_t>* const outbuffer;

        mutable std::mutex inlock;
        mutable std::mutex outlock;

        std::thread* const thread;

        std::atomic_bool doComm;

        virtual void threadRead() = 0;

    public:
        explicit ICOMPort(std::string portName, unsigned long baudRate);
        ~ICOMPort() override;

        virtual bool isReady() const;
        bool isOpen() const override;
        virtual unsigned long lastError() const;

        bool pktAvailable() const override;
        std::string interfaceType() const override;

        size_t sendData(const void* bytes, size_t length) const override;
        size_t readData(void* bytes, size_t bufferlength) const override;

        virtual bool enumSerialDevs(std::vector<std::pair<std::string, std::string>>& devs) const = 0;
    };

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

        // The interface itself
        ICOMPort* port;

    public:
        explicit COMPortChooser();

        // Renders the UI for the chooser, and returns a pointer to a valid and open interface once an interface
        // has been successfully created
        RCP_Interface* render() override;
    };
}

#endif //COMPORT_H
