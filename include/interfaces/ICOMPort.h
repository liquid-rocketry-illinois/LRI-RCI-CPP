#ifndef ICOMPORT_H
#define ICOMPORT_H

#include <vector>
#include <atomic>
#include <mutex>
#include <thread>

#include "RCP_Interface.h"
#include "utils.h"
#include "UI/TargetChooser.h"

namespace LRI::RCI {
    class ICOMPort : public RCP_Interface {
    protected:
        static constexpr int BUFFER_SIZE = 1'048'576;

        const std::string portName;
        const unsigned long baudRate;

        // Open indicates if the handle returned by windows is valid and properly configured, whereas ready can be used
        // to delay the full initialization of the UI until a later point if more setup time is needed in the thread
        std::atomic_bool portopen;
        std::atomic_bool portready;

        // The last error produced by the platform's serial device
        std::atomic_ulong lastErrorVal;

        // Buffers to store input and output bytes
        RingBuffer<uint8_t>* const inbuffer;
        RingBuffer<uint8_t>* const outbuffer;

        // Locks for those buffers
        mutable std::mutex inlock;
        mutable std::mutex outlock;

        // Pointer to the IO thread
        std::thread* const thread;

        // Set to true to stop the IO thread
        std::atomic_bool doComm;

        // The function that actually gets threaded
        virtual void threadRead() = 0;

    public:
        // COMPort needs the port name (or path on linux) as a string (COMx) and the baud rate
        explicit ICOMPort(std::string portName, unsigned long baudRate);

        // Stop the IO thread, close the device handle, and free buffers
        ~ICOMPort() override;

        // Getters for status variables
        virtual bool isReady() const;
        bool isOpen() const override;
        virtual unsigned long lastError() const;
        std::string interfaceType() const override;

        // Returns true if a full packet is ready to be read from the input buffer
        bool pktAvailable() const override;

        // Functions for writing and reading from the serial device
        size_t sendData(const void* bytes, size_t length) const override;
        size_t readData(void* bytes, size_t bufferlength) const override;
    };

    // Enumerate serial devices, implemented in either LinuxCOMPort and WindowsCOMPort
    bool enumSerialDevs(std::vector<std::pair<std::string, std::string>>& portList);

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

        // The interface itself
        ICOMPort* port;

    public:
        explicit COMPortChooser();

        // Renders the UI for the chooser, and returns a pointer to a valid and open interface once an interface
        // has been successfully created
        RCP_Interface* render() override;
    };
}

#endif // ICOMPORT_H
