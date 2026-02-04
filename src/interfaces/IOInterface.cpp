#include "interfaces/IOInterface.h"

#include <chrono>

#include <iostream>
#include "hardware/TestState.h"
namespace LRI::RCI {
    IOInterface::IOInterface() :
        doComm(true), iothread(nullptr), inbuffer(new RingBuffer<uint8_t>(BUFFER_SIZE)),
        outbuffer(new RingBuffer<uint8_t>(BUFFER_SIZE)) {
        ioLockMux.lock();
        iothread = new std::thread(&IOInterface::threadIO, this);
    }

    void IOInterface::threadIO() {
        ioLockMux.lock();
        ioInit();

        if(!isPortOpen || portOpenFail) {
            doComm = false;
            ioLockMux.unlock();
            return;
        }

        bool r_nw = false;
        while(doComm) {
            if(r_nw) {
                // Read cycle
                r_nw = false;

                // If there is no space in the buffer return
                inlock.lock();
                bool res = inbuffer->size() + TEMP_BUFFER_SIZE >= inbuffer->capacity();
                inlock.unlock();
                if(res) continue;

                uint8_t bytes[TEMP_BUFFER_SIZE];
                size_t written = 5;
                if(!readBytes(bytes, TEMP_BUFFER_SIZE, written)) continue;
                if(written == 0) continue;

                inlock.lock();
                for(size_t i = 0; i < written; i++) {
                    inbuffer->push(bytes[i]);
                }
                inlock.unlock();
            }

            else {
                // Write cycle
                r_nw = true;
                if(!TestState::getInited()) continue;

                uint8_t bytes[TEMP_BUFFER_SIZE];
                size_t written = 0;
                outlock.lock();
                for(size_t i = 0; i < TEMP_BUFFER_SIZE && !outbuffer->isEmpty(); i++, written++) {
                    bytes[i] = outbuffer->pop();
                }

                outlock.unlock();

                if(written == 0) continue;

                writeBytes(bytes, written);
            }
        }

        ioDeinit();
        ioLockMux.unlock();
    }

    void IOInterface::ioUnlock() { ioLockMux.unlock(); }

    void IOInterface::ioLock() {
        doComm = false;
        ioLockMux.lock();
    }


    bool IOInterface::getDoComm() const { return doComm.load(); }

    IOInterface::Error IOInterface::lastError() const { return {lastErrorStage.load(), lastErrorCode.load()}; }

    bool IOInterface::didPortOpenFail() const { return portOpenFail.load(); }

    bool IOInterface::isOpen() const { return isPortOpen.load(); }

    size_t IOInterface::sendData(const void* data, size_t length) const {
        // Lock the output buffer, and check if there is space to insert the data. If not, return
        outlock.lock();
        if(outbuffer->size() + length > outbuffer->capacity()) {
            outlock.unlock();
            return 0;
        }

        // Push new bytes to the buffer
        const auto _bytes = static_cast<const uint8_t*>(data);
        for(size_t i = 0; i < length; i++) {
            outbuffer->push(_bytes[i]);
        }

        // Return
        outlock.unlock();
        return length;
    }

    size_t IOInterface::readData(void* data, size_t bufferSize) const {
        size_t bytesread;
        const auto _bytes = static_cast<uint8_t*>(data);

        // Lock the input buffer and pop bytes from the buffer and place them into the output buffer
        inlock.lock();
        for(bytesread = 0; inbuffer->size() > 0 && bytesread < bufferSize; bytesread++) {
            _bytes[bytesread] = inbuffer->pop();
        }

        inlock.unlock();
        return bytesread;
    }

    bool IOInterface::pktAvailable() const {
        inlock.lock();
        bool hasData;
        if(inbuffer->size() == 0) hasData = false;
        else hasData = inbuffer->size() >= static_cast<uint32_t>(inbuffer->peek() & (~RCP_CHANNEL_MASK)) + 2;
        inlock.unlock();
        return hasData;
    }

    IOInterface::~IOInterface() {
        iothread->join();
        delete iothread;
        delete inbuffer;
        delete outbuffer;
    }

} // namespace LRI::RCI
