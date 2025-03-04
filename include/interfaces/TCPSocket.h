#ifndef LRI_CONTROL_PANEL_TCPSOCKET_H
#define LRI_CONTROL_PANEL_TCPSOCKET_H

#include "interfaces/RCP_Interface.h"
#include "SFML/Network.hpp"
#include <thread>
#include <mutex>
#include "utils.h"
#include "UI/TargetChooser.h"

namespace LRI::RCI {
    class TCPSocket : public RCP_Interface {
        static constexpr uint32_t BUFFER_SIZE = 1'000'000;
        const uint16_t port;
        const sf::IpAddress serverAddress;
        const bool isServer;

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

        explicit TCPSocket(uint16_t port, const sf::IpAddress& serverAddress = {0, 0, 0, 0});
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
        int port = 5000;
        int ip[4] = {192, 168, 254, 2};
        bool server = false;

        TCPSocket* interf = nullptr;

    public:
        explicit TCPInterfaceChooser() = default;
        RCP_Interface* render() override;
    };
}

#endif //LRI_CONTROL_PANEL_TCPSOCKET_H
