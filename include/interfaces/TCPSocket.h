#ifndef LRI_CONTROL_PANEL_TCPSOCKET_H
#define LRI_CONTROL_PANEL_TCPSOCKET_H

#include "interfaces/RCP_Interface.h"
#include "SFML/Network.hpp"
#include <thread>
#include <mutex>
#include "utils.h"
#include "UI/TargetChooser.h"

// An interface for communicating over a TCP socket. Uses the sfml network module
namespace LRI::RCI {
    class TCPSocket : public RCP_Interface {
        static constexpr uint32_t BUFFER_SIZE = 1'000'000;

        // Settings for the socket
        const uint16_t port;
        const sf::IpAddress serverAddress;

        // Used for if the host computer is the server or if the target is the server
        const bool isServer;

        // The sockets and their selectors. listen* is not used in client mode, since the client does
        // not need to listen for anything
        sf::TcpListener listensock;
        sf::TcpSocket targetsock;
        sf::SocketSelector listensel;
        sf::SocketSelector targetsel;

        // Thread for IO operations
        std::thread* thread;

        // Buffers for data
        RingBuffer<uint8_t>* inbuffer;
        RingBuffer<uint8_t>* outbuffer;

        // Mutexes for above buffers
        mutable std::mutex inlock;
        mutable std::mutex outlock;

        // Control flags between the main thread and the IO thread
        std::atomic_bool threadRun;
        std::atomic_bool open;
        std::atomic_bool ready;
        std::atomic_ulong errorStage;
        std::atomic_ulong lastErrorVal;

        // The thing that gets ran in the thread
        void runner();

        // Clean up stuff
        void close();

    public:
        // Package a program stage and error code into one
        struct Error {
            unsigned long stage;
            unsigned long value;
        };

        // Construct a new socket interface. A port is required regardless, but if the interface is a client
        // you must also provide the IP address of the server
        explicit TCPSocket(uint16_t port, const sf::IpAddress& serverAddress = {0, 0, 0, 0});
        ~TCPSocket() override;

        // Getters for state. Open is for if the interface has been constructed correctly. IsReady indicates if
        // a connection has been made and data can flow. pktAvailable is if a valid RCP packet is ready to be
        // consumed from the inbuffer
        [[nodiscard]] bool isOpen() const override;
        [[nodiscard]] bool pktAvailable() const override;
        [[nodiscard]] bool isReady() const;

        // Sending and receiving functions
        size_t sendData(const void* data, size_t length) const override;
        size_t readData(void* data, size_t bufferSize) const override;

        // Returns a human significant string of the interface
        [[nodiscard]] std::string interfaceType() const override;

        // Gets the last error
        [[nodiscard]] Error lastError();
    };

    // The chooser for a tcp interface
    class TCPInterfaceChooser final : public InterfaceChooser {
        // Default port and ip addresses
        int port = 5000;
        int ip[4] = {192, 168, 254, 2};
        bool server = false;

        TCPSocket* interf = nullptr;

    public:
        explicit TCPInterfaceChooser() = default;

        // Rendering function
        RCP_Interface* render() override;
    };
}

#endif //LRI_CONTROL_PANEL_TCPSOCKET_H
