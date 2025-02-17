#ifndef LRI_CONTROL_PANEL_TCPSOCKET_H
#define LRI_CONTROL_PANEL_TCPSOCKET_H

#include "interfaces/RCP_Interface.h"
#include "SFML/Network.hpp"
#include <thread>
#include <mutex>
#include "utils.h"
#include "UI/TargetChooser.h"
#include <iphlpapi.h>

namespace LRI::RCI {
    union IPAddress {
        struct {
            uint8_t b1;
            uint8_t b2;
            uint8_t b3;
            uint8_t b4;
        } bytes;
        uint32_t ulong;
    };

    class TCPSocket : public RCP_Interface {
        static constexpr uint32_t BUFFER_SIZE = 1'000'000;
        const DWORD adapterIndex;
        const std::string adapterName;
        const IPAddress hostaddr;
        const uint16_t port;
        const IPAddress netmask;

        ULONG NTEContext;

        sf::TcpListener listensock;
        sf::TcpSocket targetsock;
        sf::SocketSelector listensel;
        sf::SocketSelector targetsel;

        std::thread* thread;

        RingBuffer<uint8_t>* inbuffer;
        RingBuffer<uint8_t>* outbuffer;

        mutable std::mutex inlock;
        mutable std::mutex outlock;

        std::atomic_bool threadRun;
        std::atomic_bool open;
        std::atomic_bool ready;
        std::atomic_ulong errorStage;
        std::atomic_ulong lastErrorVal;

        void runner();
        void close();

    public:
        struct Error {
            unsigned long stage;
            unsigned long value;
        };

        explicit TCPSocket(DWORD adapterIndex, std::string adapterName, const IPAddress& hostaddr, uint16_t port,
                           const IPAddress& netmask = {255, 255, 255, 0});
        ~TCPSocket() override;

        [[nodiscard]] bool isOpen() const override;
        [[nodiscard]] bool pktAvailable() const override;
        [[nodiscard]] bool isReady() const;

        size_t sendData(const void* data, size_t length) const override;
        size_t readData(void* data, size_t bufferSize) const override;

        // Returns "Virtual Interface"
        [[nodiscard]] std::string interfaceType() const override;

        [[nodiscard]] Error lastError();
    };

    class TCPInterfaceChooser final : public InterfaceChooser {
        std::map<DWORD, std::string> adapters;
        int address[4] = {192, 168, 254, 1};
        int port = 12345;

        TCPSocket* interf;
        DWORD selectedAdapter;

        bool enumAdapters();
    public:
        explicit TCPInterfaceChooser();
        RCP_Interface* render() override;
    };
}

#endif //LRI_CONTROL_PANEL_TCPSOCKET_H
