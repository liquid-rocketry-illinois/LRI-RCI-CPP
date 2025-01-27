#ifndef COMPORT_H
#define COMPORT_H

#include <string>
#include <Windows.h>
#include <atomic>
#include <thread>
#include <mutex>

#include "utils.h"
#include "RCP_Interface.h"

namespace LRI::RCI {
    class COMPort : public RCP_Interface {
        static constexpr int bufferSize = 1'048'576;

        char* const portname;
        DWORD const baudrate;
        HANDLE const port;

        // Open indicates if the handle returned by windows is valid and properly configured, whereas ready can be used
        // to delay the full initialization of the UI until a later point if more setup time is needed in the thread
        bool open = false;
        std::atomic_bool ready = false;

        // The last error produced by Windows from IO with the serial device
        std::atomic_ulong lastErrorVal;

        // Buffers to store input and output bytes
        RCI::RingBuffer<uint8_t>* inbuffer;
        RCI::RingBuffer<uint8_t>* outbuffer;

        // Locks for those buffers
        mutable std::mutex inlock;
        mutable std::mutex outlock;

        // Pointer to the IO thread
        std::thread* thread;

        // Set to true to stop the IO thread
        std::atomic_bool doComm;

        void threadRead();

    public:
        // COMPort needs the port name as a string (COMx) and the baud rate
        explicit COMPort(const char* portname, const DWORD& baudrate);
        ~COMPort() override;

        // Stop the IO thread, close the device handle, and free buffers
        bool close();

        // See above for difference between open and ready
        bool isOpen() const override;
        bool isReady() const;

        // Returns true if a full packet is ready to be read from the input buffer
        bool pktAvailable() const override;

        // Query last error
        DWORD lastError() const;

        // A display string representing the port (COMx @ y baud)
        std::string interfaceType() const override;

        // Functions for writing and reading from the serial device
        size_t sendData(const void* bytes, size_t length) const override;
        size_t readData(void* bytes, size_t bufferlength) const override;
    };
}
#endif //COMPORT_H
