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
        class RingBuffer;
        static constexpr int bufferSize = 1024;

        char* portname;
        HANDLE port;
        bool open = false;
        std::atomic_ulong lastErrorVal;

        RCI::RingBuffer<uint8_t>* buffer;
        std::atomic_bool read;
        std::thread* thread;
        mutable std::mutex dataAccess;

        void threadRead();

    public:
        explicit COMPort(const char* portname, DWORD baudrate);
        ~COMPort() override;
        bool close();
        bool isOpen() const override;
        bool pktAvailable() const override;
        DWORD lastError() const;
        size_t sendData(const void* bytes, size_t length) const override;
        size_t readData(void* bytes, size_t buffersize) const override;
        std::string interfaceType() const override;
    };
}
#endif //COMPORT_H
