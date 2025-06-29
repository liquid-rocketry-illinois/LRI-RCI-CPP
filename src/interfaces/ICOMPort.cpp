#include "interfaces/ICOMPort.h"
#include "improgress.h"

namespace LRI::RCI {
    ICOMPort::ICOMPort(std::string portName, unsigned long baudRate)
        : portName(std::move(portName)),
          baudRate(baudRate), portopen(true), portready(false),
          lastErrorVal(0),
          inbuffer(new RingBuffer<uint8_t>(BUFFER_SIZE)),
          outbuffer(new RingBuffer<uint8_t>(BUFFER_SIZE)),
          thread(new std::thread(&ICOMPort::threadRead, this)),
          doComm(true) {
    }

    ICOMPort::~ICOMPort() {
        doComm = false;
        portopen = false;
        portready = false;

        if (thread) thread->join();
        delete thread;
        delete inbuffer;
        delete outbuffer;
    }

    bool ICOMPort::isOpen() const { return portopen; }

    bool ICOMPort::isReady() const { return portready; }

    unsigned long ICOMPort::lastError() const { return lastErrorVal; }

    bool ICOMPort::pktAvailable() const {
        inlock.lock();
        bool hasData;
        if (inbuffer->size() == 0) hasData = false;
        else hasData = inbuffer->size() >= (inbuffer->peek() & (~RCP_CHANNEL_MASK)) + 2;
        inlock.unlock();
        return hasData;
    }

    std::string ICOMPort::interfaceType() const {
        return "Serial Port (" + portName + " @ " + std::to_string(baudRate) + " baud)";
    }

    size_t ICOMPort::sendData(const void *bytes, const size_t length) const {
        // Lock the output buffer, and check if there is space to insert the data. If not, return
        outlock.lock();
        if (outbuffer->size() + length > outbuffer->capacity()) {
            outlock.unlock();
            return 0;
        }

        // Push new bytes to the buffer
        const auto _bytes = static_cast<const uint8_t *>(bytes);
        for (size_t i = 0; i < length; i++) {
            outbuffer->push(_bytes[i]);
        }

        // Return
        outlock.unlock();
        return length;
    }

    size_t ICOMPort::readData(void *bytes, size_t bufferlength) const {
        int bytesread;
        const auto _bytes = static_cast<uint8_t *>(bytes);

        // Lock the input buffer and pop bytes from the buffer and place them into the output buffer
        inlock.lock();
        for (bytesread = 0; inbuffer->size() > 0 && bytesread < bufferlength; bytesread++) {
            _bytes[bytesread] = inbuffer->pop();
        }

        inlock.unlock();
        return bytesread;
    }

    COMPortChooser::COMPortChooser()
        : selectedPort(0), error(false), baud(115200), port(nullptr) { enumSerialDevs(portlist); }

    RCP_Interface *COMPortChooser::render() {
        ImGui::PushID("COMPortChooser");
        ImGui::PushID(classid);

        bool disable = port;
        if (disable) ImGui::BeginDisabled();

        // It has a dropdown for which device, as listed in enumSerial()
        ImGui::Text("Choose Serial Port: ");
        ImGui::SameLine();
        if (portlist.empty()) ImGui::Text("No Ports Detected");
        else if (ImGui::BeginCombo("##portselectcombo", portlist[selectedPort].second.c_str())) {
            for (size_t i = 0; i < portlist.size(); i++) {
                ImGui::PushID(i);
                bool selected = i == selectedPort;
                if (ImGui::Selectable((portlist[i]).second.c_str(), &selected)) selectedPort = i;
                if (selected) ImGui::SetItemDefaultFocus();
                ImGui::PopID();
            }

            ImGui::EndCombo();
        }

        if (ImGui::Button("Refresh List")) {
            selectedPort = 0;
            enumSerialDevs(portlist);
        }

        // Input for baud rate
        ImGui::Text("Baud Rate: ");
        ImGui::SameLine();
        ImGui::SetNextItemWidth(scale(100));
        ImGui::InputInt("##comportchooserbaudinput", &baud);
        if (baud < 0) baud = 0;
        else if (baud > 500'000) baud = 500'000;

        if (portlist.empty()) ImGui::BeginDisabled();
        if (ImGui::Button("Connect")) {
            // If connect, then create the COMPort
            // port = new COMPort((R"(\\.\)" + portlist[selectedPort].first).c_str(), baud);
        }
        if (portlist.empty()) ImGui::EndDisabled();
        if (disable) ImGui::EndDisabled();

        // If the port failed to allocate then return
        if (!port) {
            ImGui::PopID();
            ImGui::PopID();
            return nullptr;
        }

        // If the port allocated but did not open then show an error
        if (!port->isOpen()) {
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1, 0, 0, 1));
            ImGui::Text("Error Connecting to Serial Port (%u)", port->lastError());
            ImGui::PopStyleColor();

            ImGui::SameLine();
            if (ImGui::Button("OK##comportchoosererror")) {
                delete port;
                ImGui::PopID();
                ImGui::PopID();
                port = nullptr;
            }
        }

        // While the port is readying, don't return it just yet and display a loading spinner
        else if (!port->isReady()) {
            ImGui::SameLine();
            ImGui::Spinner("##comportchooserspinner", 8, 1, WModule::REBECCA_PURPLE);
        } else {
            ImGui::PopID();
            ImGui::PopID();
            return port;
        }

        ImGui::PopID();
        ImGui::PopID();
        return nullptr;
    }
} // namespace LRI::RCI
