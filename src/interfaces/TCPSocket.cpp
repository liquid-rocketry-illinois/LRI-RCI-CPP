#include "interfaces/TCPSocket.h"
#include <iphlpapi.h>

#include <utility>
#include "UI/TargetChooser.h"
#include "improgress.h"

namespace LRI::RCI {
    TCPSocket::TCPSocket(DWORD adapterIndex, std::string adapterName, const IPAddress& hostaddr, uint16_t port,
                         const IPAddress& netmask)
            :
            adapterIndex(adapterIndex), adapterName(std::move(adapterName)), hostaddr(hostaddr), port(port),
            netmask(netmask), NTEContext(0), thread(nullptr), inbuffer(nullptr), outbuffer(nullptr), threadRun(false),
            open(false), ready(false), lastErrorVal(0) {

        DWORD error = AddIPAddress(hostaddr.ulong, netmask.ulong, adapterIndex, &NTEContext, nullptr);

        if(error != NO_ERROR) {
            errorStage = 1;
            lastErrorVal = error;
            return;
        }

        sf::Socket::Status listenStat = listensock.listen(port);
        if(listenStat != sf::Socket::Status::Done) {
            errorStage = 2;
            lastErrorVal = -1;
            return;
        }

        listensel.add(listensock);
        targetsel.add(targetsock);

        inbuffer = new RingBuffer<uint8_t>(BUFFER_SIZE);
        outbuffer = new RingBuffer<uint8_t>(BUFFER_SIZE);
        threadRun = true;
        open = true;
        thread = new std::thread(&TCPSocket::runner, this);
    }

    TCPSocket::~TCPSocket() {
        close();
    }

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

    std::string TCPSocket::interfaceType() const {
        return "TCP Socket Interface";
    }

    TCPSocket::Error TCPSocket::lastError() {
        return {errorStage.load(), lastErrorVal.load()};
    }

    bool TCPSocket::isOpen() const {
        return open.load();
    }

    bool TCPSocket::isReady() const {
        return ready.load();
    }

    void TCPSocket::runner() {
        bool r_nw = false;

        while(threadRun) {
            if(!open) {
                if(listensel.wait(sf::seconds(0.25f))) {
                    if(listensock.accept(targetsock) != sf::Socket::Status::Done) {
                        errorStage = 3;
                        lastErrorVal = -1;
                        threadRun = false;
                        return;
                    }

                    ready = true;
                    errorStage = 0;
                    lastErrorVal = 0;
                }

                continue;
            }

            if(r_nw) {
                r_nw = false;
                inlock.lock();
                bool skip = inbuffer->size() + 128 > inbuffer->capacity();
                inlock.unlock();
                if(skip) continue;

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
                    }

                    inlock.unlock();
                }

            }

            else {
                r_nw = true;

                outlock.lock();
                bool skip = !outbuffer->isEmpty();
                if(skip) {
                    outlock.unlock();
                    continue;
                }

                uint8_t bytes[128];
                size_t toSend;

                for(toSend = 0; !outbuffer->isEmpty() && toSend < 128; toSend++) bytes[toSend] = outbuffer->pop();

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

    void TCPSocket::close() {
        threadRun = false;
        if(thread) thread->join();
        delete thread;
        thread = nullptr;

        listensock.close();

        delete inbuffer;
        inbuffer = nullptr;

        delete outbuffer;
        outbuffer = nullptr;

        if(NTEContext != 0) {
            DWORD error = DeleteIPAddress(NTEContext);
            if(error != NO_ERROR) {
                errorStage = -1;
                lastErrorVal = error;
            }

            else {
                errorStage = 0;
                lastErrorVal = 0;
            }
        }
    }

    TCPInterfaceChooser::TCPInterfaceChooser()
            : port(0), interf(nullptr), selectedAdapter(-1) {
        enumAdapters();
    }

    bool TCPInterfaceChooser::enumAdapters() {
        auto* info = (IP_ADAPTER_INFO*) malloc(sizeof(IP_ADAPTER_INFO));
        ULONG outbuflen = sizeof(IP_ADAPTER_INFO);

        if(GetAdaptersInfo(info, &outbuflen) != ERROR_SUCCESS) {
            free(info);
            info = (IP_ADAPTER_INFO*) malloc(outbuflen);

            if(GetAdaptersInfo(info, &outbuflen) != ERROR_SUCCESS) {
                return false;
            }
        }

        PIP_ADAPTER_INFO adapter = info;
        while(adapter) {
            bool doAdd = true;
            std::string display = std::string(adapter->Description) + " (";

            switch(adapter->Type) {
                case MIB_IF_TYPE_ETHERNET:
                    display += "Ethernet";
                    break;

                case MIB_IF_TYPE_LOOPBACK:
                    doAdd = false;
                    break;

                case IF_TYPE_IEEE80211:
                    display += "802.11";
                    break;

                default:
                    display += "Unknown";
                    break;
            }

            if(doAdd) {
                adapters[adapter->Index] = display + ")";

                if(selectedAdapter == -1) selectedAdapter = adapter->Index;
            }
            adapter = adapter->Next;
        }

        free(info);
        return true;
    }

    IPAddress toAddr(int* asInts) {
        return {
                static_cast<uint8_t>(asInts[0]),
                static_cast<uint8_t>(asInts[1]),
                static_cast<uint8_t>(asInts[2]),
                static_cast<uint8_t>(asInts[3]),
        };
    }

    RCP_Interface* TCPInterfaceChooser::render() {
        bool isnull = interf == nullptr;
        if(!isnull) ImGui::BeginDisabled();
        if(adapters.empty()) {
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1, 0, 0, 1));
            ImGui::Text("Failed to enumerate network adapters!");
            ImGui::PopStyleColor();
            return nullptr;
        }

        ImGui::Text("Choose network adapter: ");
        ImGui::SameLine();
        ImGui::SetNextItemWidth(scale(300));
        if(ImGui::BeginCombo("##adapterchooser", adapters[selectedAdapter].c_str())) {
            for(const auto& [index, info] : adapters) {
                bool selected = index == selectedAdapter;
                if(ImGui::Selectable((info + "##portselectcombo").c_str(), &selected))
                    selectedAdapter = index;
                if(selected) ImGui::SetItemDefaultFocus();
            }

            ImGui::EndCombo();
        }

        ImGui::Text("IP Address:  ");
        ImGui::SameLine();
        ImGui::SetNextItemWidth(scale(200));
        ImGui::InputInt4("##ipinput", address);
        for(int& part : address) {
            if(part < 0) part = 0;
            else if(part > 255) part = 255;
        }

        ImGui::Text("Listen port: ");
        ImGui::SameLine();
        ImGui::SetNextItemWidth(scale(48));
        ImGui::InputInt("##portinput", &port, 0);
        if(port < 0) port = 0;
        else if(port > 65535) port = 65535;

        if(ImGui::Button("Begin Hosting")) {
            interf = new TCPSocket(selectedAdapter, adapters[selectedAdapter], toAddr(address), port);
        }

        ImGui::SameLine();
        if(ImGui::Button("Refresh Adapters##tcprefreshadapters")) {
            adapters.clear();
            enumAdapters();
        }

        if(!isnull) ImGui::EndDisabled();

        if(interf == nullptr) return nullptr;

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

        if(!interf->isReady()) {
            ImGui::SameLine();
            ImGui::Text("Waiting for connection");
            ImGui::SameLine();
            ImGui::Spinner("##tcpwaitspinner", 8, 1, BaseUI::REBECCA_PURPLE);

            if(ImGui::Button("Cancel##tcpcancel")) {
                delete interf;
                interf = nullptr;
            }
            return nullptr;
        }

        return interf;
    }

}