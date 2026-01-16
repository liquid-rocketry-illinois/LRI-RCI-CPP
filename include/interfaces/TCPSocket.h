#ifndef LRI_CONTROL_PANEL_TCPSOCKET_H
#define LRI_CONTROL_PANEL_TCPSOCKET_H

#include "SFML/Network.hpp"

#include "interfaces/IOInterface.h"
#include "utils.h"

// An interface for communicating over a TCP socket. Uses the sfml network module
namespace LRI::RCI {
    class TCPSocket : public IOInterface {
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

    public:
        // Construct a new socket interface. A port is required regardless, but if the interface is a client
        // you must also provide the IP address of the server
        explicit TCPSocket(uint16_t port, const sf::IpAddress& serverAddress = {0, 0, 0, 0});
        ~TCPSocket() override;

        // Returns a human significant string of the interface
        [[nodiscard]] std::string interfaceType() const override;

        void ioInit() override;
        bool writeBytes(const uint8_t* bytes, size_t length) override;
        bool readBytes(uint8_t* bytes, size_t bufLength, size_t& written) override;
        void ioDeinit() override;
    };
} // namespace LRI::RCI

#endif // LRI_CONTROL_PANEL_TCPSOCKET_H
