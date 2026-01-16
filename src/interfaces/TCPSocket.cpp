#include "interfaces/TCPSocket.h"

#include "hardware/TestState.h"

namespace LRI::RCI {
    // An RCP interface over TCP

    // For comparisons in the constructor to determine if the interface should be a server or client
    const auto DEFAULT_IP = sf::IpAddress(0, 0, 0, 0);

    // Initialize all the stuff
    TCPSocket::TCPSocket(uint16_t port, const sf::IpAddress& serverAddress) :
        port(port), serverAddress(serverAddress), isServer(serverAddress != DEFAULT_IP) {
        ioUnlock();
    }

     TCPSocket::~TCPSocket() {
        ioLock();
    }

    // Return a human readable string describing the interface
    std::string TCPSocket::interfaceType() const {
        return std::string("TCP ") + (isServer ? "Client: " + serverAddress.toString() + ": " : "Server: ") +
            std::to_string(port);
    }

    void TCPSocket::ioInit() {
        // If the interface is the server, it should start listening for connections
        if(isServer) {
            sf::Socket::Status listenStat = listensock.listen(port);
            if(listenStat != sf::Socket::Status::Done) {
                lastErrorStage = 2;
                lastErrorCode = -1;
                portOpenFail = true;
                return;
            }

            listensel.add(listensock);
        }

        targetsock.setBlocking(false);

        while(!isPortOpen && getDoComm()) {
            // If the interface is a server, wait for a connection request
            if(isServer && listensel.wait(sf::seconds(0.25f))) {
                // If a connection request has been received, accept the socket and move on to data transfer
                if(listensock.accept(targetsock) != sf::Socket::Status::Done) {
                    lastErrorStage = 3;
                    lastErrorCode = -1;
                    portOpenFail = true;
                    return;
                }

                isPortOpen = true;
                lastErrorStage = 0;
                lastErrorCode = 0;
                targetsel.add(targetsock);
            }

            // If the interface is not a server, connect to a server
            else {
                auto stat = targetsock.connect(serverAddress, port);

                if(stat == sf::Socket::Status::NotReady) continue;

                if(stat != sf::Socket::Status::Done) {
                    lastErrorStage = 3;
                    lastErrorCode = -1;
                    portOpenFail = true;
                    return;
                }

                using namespace std::chrono_literals;
                std::this_thread::sleep_for(3000ms); // Seems to help with stuff

                uint8_t tempdata[33];
                size_t temp;
                stat = targetsock.receive(tempdata, 33, temp);
                // For some reason ser2net sends 33 junk bytes when connected

                if(stat != sf::Socket::Status::Done) {
                    lastErrorStage = 3;
                    lastErrorCode = -2;
                    portOpenFail = true;
                    targetsock.disconnect();
                    return;
                }

                isPortOpen = true;
                lastErrorStage = 0;
                lastErrorCode = 0;
                targetsel.add(targetsock);
            }
        }

        targetsock.setBlocking(true);
    }

    bool TCPSocket::writeBytes(const uint8_t* bytes, size_t length) {
        sf::Socket::Status stat = targetsock.send(bytes, length);
        if(stat != sf::Socket::Status::Done) {
            lastErrorStage = 5;
            lastErrorCode = stat == sf::Socket::Status::Disconnected ? 1 : 2;
            return false;
        }

        return true;
    }

    bool TCPSocket::readBytes(uint8_t* bytes, size_t bufLength, size_t& written) {
        if(targetsel.wait(sf::milliseconds(100))) {
            sf::Socket::Status stat = targetsock.receive(bytes, bufLength, written);
            if(stat != sf::Socket::Status::Done) {
                lastErrorStage = 6;
                lastErrorCode = stat == sf::Socket::Status::Disconnected ? 1 : 2;
                return false;
            }
        }

        return true;
    }

    void TCPSocket::ioDeinit() {
        if(isServer) listensock.close();
        targetsock.disconnect();
    }
} // namespace LRI::RCI
