#include "interfaces/TCPSocket.h"
#include <iostream>
#include "UI/TargetChooser.h"
#include "hardware/TestState.h"
#include "improgress.h"

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

    // Chooser for the interface. Just needs port, client/server, and server ip address if client
    RCP_Interface* TCPInterfaceChooser::render() {
        ImGui::PushID("TCPSocketChooser");
        ImGui::PushID(classid);

        bool isnull = interf == nullptr;
        if(!isnull) ImGui::BeginDisabled();
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0, 1, 0, 1));
        ImGui::Text("Make sure to set the computer's static IP in control panel!");
        ImGui::PopStyleColor();

        // Buttons to pick between client or server
        bool tempserver = server;
        if(tempserver) ImGui::BeginDisabled();
        if(ImGui::Button("Server")) server = true;
        if(tempserver) ImGui::EndDisabled();

        ImGui::SameLine();
        if(!tempserver) ImGui::BeginDisabled();
        if(ImGui::Button("Client")) server = false;
        if(!tempserver) ImGui::EndDisabled();

        // If in client mode, ask for ip address
        if(!tempserver) {
            ImGui::Text("Server Address: ");
            ImGui::SameLine();
            ImGui::SetNextItemWidth(scale(200));
            ImGui::InputInt4("##serveraddrinput", ip);
            for(int& i : ip) {
                if(i < 0) i = 0;
                else if(i > 255) i = 255;
            }
        }

        // Port input
        ImGui::Text("Port: ");
        ImGui::SameLine();
        ImGui::SetNextItemWidth(scale(48));
        ImGui::InputInt("##portinput", &port, 0);
        if(port < 0) port = 0;
        else if(port > 65535) port = 65535;

        // Once confirm is pushed, create the interface and begin waiting for a connection
        if(ImGui::Button(tempserver ? "Begin Hosting" : "Connect")) {
            interf =
                new TCPSocket(port, tempserver ? sf::IpAddress(0, 0, 0, 0) : sf::IpAddress(ip[0], ip[1], ip[2], ip[3]));
        }
        if(!isnull) ImGui::EndDisabled();

        // If the interface is null, keep rendering
        if(interf == nullptr) {
            ImGui::PopID();
            ImGui::PopID();
            return nullptr;
        }

        // If the interface has been created but did not open properly, display error and continue rendering
        if(interf->didPortOpenFail()) {
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1, 0, 0, 1));
            TCPSocket::Error error = interf->lastError();
            ImGui::Text("Error opening TCP socket: stage %lu, code %lu", error.stage, error.code);
            ImGui::PopStyleColor();

            ImGui::SameLine();
            if(ImGui::Button("OK")) {
                delete interf;
                ImGui::PopID();
                ImGui::PopID();
                interf = nullptr;
            }

            ImGui::PopID();
            ImGui::PopID();
            return nullptr;
        }

        // While the interface is open but not ready, keep waiting for the connection to be established
        if(!interf->isOpen()) {
            ImGui::SameLine();
            ImGui::Text("Waiting for connection");
            ImGui::SameLine();
            ImGui::Spinner("##tcpwaitspinner", 8, 1, WModule::REBECCA_PURPLE);

            if(ImGui::Button("Cancel")) {
                delete interf;
                ImGui::PopID();
                ImGui::PopID();
                interf = nullptr;
            }

            ImGui::PopID();
            ImGui::PopID();
            return nullptr;
        }

        ImGui::PopID();
        ImGui::PopID();
        // Once the interface is ready to go, return it to the TargetChooser
        return interf;
    }
} // namespace LRI::RCI
