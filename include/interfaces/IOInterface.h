#ifndef IOINTERFACE_H
#define IOINTERFACE_H

#include <atomic>
#include <mutex>
#include <thread>

#include "RCP_Interface.h"
#include "utils.h"

namespace LRI::RCI {
  class IOInterface : public RCP_Interface {
    static constexpr uint8_t BUFFER_SIZE = 1'048'576;
    static constexpr size_t TEMP_BUFFER_SIZE = 128;

    std::atomic_bool doComm;
    std::mutex ioLock;
    std::thread* iothread;

    RingBuffer<uint8_t>* const inbuffer;
    RingBuffer<uint8_t>* const outbuffer;

    mutable std::mutex inlock;
    mutable std::mutex outlock;

    void threadIO();

  protected:
    std::atomic_bool isPortOpen;
    std::atomic_bool portOpenFail;
    std::atomic_ulong lastErrorStage;
    std::atomic_ulong lastErrorCode;

    virtual void ioInit() = 0;
    virtual bool writeBytes(const uint8_t* bytes, size_t length) = 0;
    virtual bool readBytes(uint8_t* bytes, size_t bufLength, size_t& written) = 0;
    virtual void ioDeinit() = 0;

    void ioUnlock();

    [[nodiscard]] bool getDoComm() const;

  public:
    struct Error {
      unsigned long stage;
      unsigned long code;
    };

    [[nodiscard]] Error lastError() const;
    [[nodiscard]] bool didPortOpenFail() const;

    [[nodiscard]] bool isOpen() const override;

    size_t sendData(const void* data, size_t length) const override;
    size_t readData(void* data, size_t bufferSize) const override;

    [[nodiscard]] bool pktAvailable() const override;

    IOInterface();
    ~IOInterface() override;
  };
}

#endif //IOINTERFACE_H
