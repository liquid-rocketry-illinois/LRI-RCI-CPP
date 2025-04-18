#include "interfaces/TCPSocket.h"
#include "hardware/TestState.h"
#include "UI/TargetChooser.h"
#include "improgress.h"
#include <iostream>

namespace LRI::RCI {
    // An RCP interface over TCP

    // For comparisons in the constructor to determine if the interface should be a server or client
    const auto DEFAULT_IP = sf::IpAddress(0, 0, 0, 0);

    // Initialize all the stuff
    TCPSocket::TCPSocket(uint16_t port, const sf::IpAddress& serverAddress) :
        port(port), serverAddress(serverAddress), isServer(serverAddress != DEFAULT_IP), thread(nullptr),
        inbuffer(nullptr), outbuffer(nullptr), threadRun(false), open(false), ready(false),
        lastErrorVal(0) {
        // If the interface is the server, it should start listening for connections
        if(isServer) {
            sf::Socket::Status listenStat = listensock.listen(port);
            if(listenStat != sf::Socket::Status::Done) {
                errorStage = 2;
                lastErrorVal = -1;
                return;
            }

            listensel.add(listensock);
        }

        // Initialize everything else
        inbuffer = new RingBuffer<uint8_t>(BUFFER_SIZE);
        outbuffer = new RingBuffer<uint8_t>(BUFFER_SIZE);
        threadRun = true;
        open = true;
        thread = new std::thread(&TCPSocket::runner, this);
    }

    // Destructor just calls close
    TCPSocket::~TCPSocket() {
        close();
    }

    // The next 3 functions are the exact same as in COMPort. Literally directly copied

    bool TCPSocket::pktAvailable() const {
        inlock.lock();
        bool hasData;
        if(inbuffer->size() == 0) hasData = false;
        else hasData = inbuffer->size() >= (inbuffer->peek() & (~RCP_CHANNEL_MASK)) + 2;
        inlock.unlock();
        return hasData;
    }

    size_t TCPSocket::sendData(const void* data, size_t length) const {
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

    size_t TCPSocket::readData(void* data, size_t bufferSize) const {
        int bytesread;
        const auto _bytes = static_cast<uint8_t*>(data);

        // Lock the input buffer and pop bytes from the buffer and place them into the output buffer
        inlock.lock();
        for(bytesread = 0; inbuffer->size() > 0 && bytesread < bufferSize; bytesread++) {
            _bytes[bytesread] = inbuffer->pop();
        }

        inlock.unlock();
        return bytesread;
    }

    // Return a human readable string describing the interface
    std::string TCPSocket::interfaceType() const {
        return std::string("TCP ") + (isServer
            ? "Client: " + serverAddress.toString() + ": "
            : "Server: ") + std::to_string(port);
    }

    // Return the last error stage and code
    TCPSocket::Error TCPSocket::lastError() {
        return {errorStage.load(), lastErrorVal.load()};
    }

    bool TCPSocket::isOpen() const {
        return open.load();
    }

    bool TCPSocket::isReady() const {
        return ready.load();
    }

    // The actual threaded function
    void TCPSocket::runner() {
        bool r_nw = false;

        // This is used to wait for incoming connections or for the connection to the server to be established
        while(!ready && threadRun) {
            // If the interface is a server, wait for a connection request
            if(isServer && listensel.wait(sf::seconds(0.25f))) {
                // If a connection request has been received, accept the socket and move on to data transfer
                if(listensock.accept(targetsock) != sf::Socket::Status::Done) {
                    errorStage = 3;
                    lastErrorVal = -1;
                    threadRun = false;
                    open = false;
                    return;
                }

                ready = true;
                errorStage = 0;
                lastErrorVal = 0;
                targetsel.add(targetsock);
            }

            // If the interface is not a server, connect to a server
            else {
                auto stat = targetsock.connect(serverAddress, port, sf::seconds(5));
                if(stat != sf::Socket::Status::Done) {
                    errorStage = 3;
                    lastErrorVal = -1;
                    threadRun = false;
                    open = false;
                    return;
                }
                using namespace std::chrono_literals;
                std::this_thread::sleep_for(3000ms); // Seems to help with stuff

                uint8_t tempdata[33];
                size_t temp;
                stat = targetsock.receive(tempdata, 33, temp);
                // For some reason ser2net sends 33 junk bytes when connected

                if(stat != sf::Socket::Status::Done) {
                    errorStage = 3;
                    lastErrorVal = -2;
                    threadRun = false;
                    open = false;
                    targetsock.disconnect();
                    return;
                }

                ready = true;
                errorStage = 0;
                lastErrorVal = 0;
                targetsel.add(targetsock);
            }
        }

        // Now that a connection has been established, start transferring data
        while(threadRun) {
            // On the read cycle...
            if(r_nw) {
                r_nw = false;
                inlock.lock();
                bool skip = inbuffer->size() + 128 > inbuffer->capacity();
                inlock.unlock();
                if(skip) continue;

                // If data is available, read it and push it into the buffer
                if(targetsel.wait(sf::seconds(0.25f))) {
                    uint8_t data[128];
                    size_t received;
                    sf::Socket::Status status;
                    if((status = targetsock.receive(data, 128, received)) != sf::Socket::Status::Done) {
                        targetsock.disconnect();
                        open = false;
                        ready = false;
                        threadRun = false;
                        errorStage = 4;
                        lastErrorVal = status == sf::Socket::Status::Disconnected ? -1 : -2;
                        return;
                    }

                    inlock.lock();
                    for(int i = 0; i < received; i++) {
                        inbuffer->push(data[i]);
                        // std::cout << "rcv: " << std::hex << (int)data[i] << std::endl;
                    }

                    inlock.unlock();
                }
            }

            // On the write cycle...
            else {
                r_nw = true;
                if(!TestState::getInited()) continue;

                outlock.lock();
                bool skip = outbuffer->isEmpty();
                if(skip) {
                    outlock.unlock();
                    continue;
                }

                uint8_t bytes[128];
                size_t toSend;

                for(toSend = 0; !outbuffer->isEmpty() && toSend < 128; toSend++) {
                    bytes[toSend] = outbuffer->pop();
                    // std::cout << "send: " << std::hex << (int)bytes[toSend] << std::endl;
                }
                outlock.unlock();

                // Pop bytes from the buffer and send over the socket
                sf::Socket::Status status = targetsock.send(bytes, toSend);

                if(status != sf::Socket::Status::Done) {
                    errorStage = 5;
                    lastErrorVal = status == sf::Socket::Status::Disconnected ? -1 : -2;
                    targetsock.disconnect();
                    open = false;
                    ready = false;
                    threadRun = false;
                    return;
                }
            }
        }
    }

    // Cleanup the interface
    void TCPSocket::close() {
        threadRun = false;
        if(thread) thread->join();
        delete thread;
        thread = nullptr;

        if(isServer) listensock.close();
        targetsock.disconnect();

        delete inbuffer;
        inbuffer = nullptr;

        delete outbuffer;
        outbuffer = nullptr;
    }

    // Chooser for the interface. Just needs port, client/server, and server ip address if client
    RCP_Interface* TCPInterfaceChooser::render() {
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
            interf = new TCPSocket(port, tempserver
                                   ? sf::IpAddress(0, 0, 0, 0)
                                   : sf::IpAddress(ip[0], ip[1], ip[2], ip[3]));
        }
        if(!isnull) ImGui::EndDisabled();

        // If the interface is null, keep rendering
        if(interf == nullptr) return nullptr;

        // If the interface has been created but did not open properly, display error and continue rendering
        if(!interf->isOpen()) {
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1, 0, 0, 1));
            TCPSocket::Error error = interf->lastError();
            ImGui::Text("Error opening TCP socket: stage %lu, code %lu", error.stage, error.value);
            ImGui::PopStyleColor();

            ImGui::SameLine();
            if(ImGui::Button("OK##tcperrorok")) {
                delete interf;
                interf = nullptr;
            }

            return nullptr;
        }

        // While the interface is open but not ready, keep waiting for the connection to be established
        if(!interf->isReady()) {
            ImGui::SameLine();
            ImGui::Text("Waiting for connection");
            ImGui::SameLine();
            ImGui::Spinner("##tcpwaitspinner", 8, 1, WModule::REBECCA_PURPLE);

            if(ImGui::Button("Cancel##tcpcancel")) {
                delete interf;
                interf = nullptr;
            }
            return nullptr;
        }

        // Once the interface is ready to go, return it to the TargetChooser
        return interf;
    }
}
