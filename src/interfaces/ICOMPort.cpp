#include "interfaces/COMPort.h"

namespace LRI::RCI {
    ICOMPort::ICOMPort(std::string portName, unsigned long baudRate) :
        portName(std::move(portName)), baudRate(baudRate), open(true), ready(false), lastErrorVal(0),
        inbuffer(new RingBuffer<uint8_t>(BUFFER_SIZE)), outbuffer(new RingBuffer<uint8_t>(BUFFER_SIZE)),
        thread(new std::thread(&ICOMPort::threadRead, this)), doComm(false) {}

    ICOMPort::~ICOMPort() {
        doComm = false;
        open = false;
        ready = false;

        if(thread) thread->join();
        delete thread;
        delete inbuffer;
        delete outbuffer;
    }

    bool ICOMPort::isOpen() const { return open; }

    bool ICOMPort::isReady() const { return ready; }

    unsigned long ICOMPort::lastError() const { return lastErrorVal; }

    bool ICOMPort::pktAvailable() const {
        inlock.lock();
        bool hasData;
        if(inbuffer->size() == 0) hasData = false;
        else hasData = inbuffer->size() >= (inbuffer->peek() & (~RCP_CHANNEL_MASK)) + 2;
        inlock.unlock();
        return hasData;
    }

    std::string ICOMPort::interfaceType() const {
        return "Serial Port (" + portName + " @ " + std::to_string(baudRate) + " baud)";
    }




} // namespace LRI::RCI
