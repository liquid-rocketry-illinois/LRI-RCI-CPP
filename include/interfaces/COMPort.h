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
        static constexpr int bufferSize = 1'048'576;

        char* const portname;
        DWORD const baudrate;
        HANDLE const port;

        bool open = false;
        std::atomic_bool ready = false;
        std::atomic_ulong lastErrorVal;

        RCI::RingBuffer<uint8_t>* inbuffer;
        RCI::RingBuffer<uint8_t>* outbuffer;
        mutable std::mutex inlock;
        mutable std::mutex outlock;

        std::thread* thread;
        std::atomic_bool doComm;

        void threadRead();

    public:
        explicit COMPort(const char* portname, const DWORD& baudrate);
        ~COMPort() override;

        bool close();
        bool isOpen() const override;
        bool isReady() const;
        bool pktAvailable() const override;
        DWORD lastError() const;
        std::string interfaceType() const override;

        size_t sendData(const void* bytes, size_t length) const override;
        size_t readData(void* bytes, size_t bufferlength) const override;
    };
}
#endif //COMPORT_H
