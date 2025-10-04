#include "interfaces/COMPort.h"

#include "UI/WModule.h"
#include "improgress.h"
#include "utils.h"

namespace LRI::RCI {
    // The COMPort chooser will enumerate all available serial devices to be picked from
    COMPortChooser::COMPortChooser() : selectedPort(0), error(false), baud(115200), port(nullptr) {
        COMPort::enumSerialDevs(portlist);
    }

    // The COMPort chooser rendering function
    RCP_Interface* COMPortChooser::render() {
        ImGui::PushID("COMPortChooser");
        ImGui::PushID(classid);

        bool disable = port;
        if(disable) ImGui::BeginDisabled();

        // It has a dropdown for which device, as listed in enumSerial()
        ImGui::Text("Choose Serial Port: ");
        ImGui::SameLine();
        if(portlist.empty()) ImGui::Text("No Ports Detected");
        else if(ImGui::BeginCombo("##portselectcombo", portlist[selectedPort].second.c_str())) {
            for(size_t i = 0; i < portlist.size(); i++) {
                ImGui::PushID(i);
                bool selected = i == selectedPort;
                if(ImGui::Selectable((portlist[i]).second.c_str(), &selected)) selectedPort = i;
                if(selected) ImGui::SetItemDefaultFocus();
                ImGui::PopID();
            }

            ImGui::EndCombo();
        }

        if(ImGui::Button("Refresh List")) {
            selectedPort = 0;
            COMPort::enumSerialDevs(portlist);
        }

        // Input for baud rate
        ImGui::Text("Baud Rate: ");
        ImGui::SameLine();
        ImGui::SetNextItemWidth(scale(100));
        ImGui::InputInt("##comportchooserbaudinput", &baud);
        if(baud < 0) baud = 0;
        else if(baud > 500'000) baud = 500'000;

        if(portlist.empty()) ImGui::BeginDisabled();
        if(ImGui::Button("Connect")) {
            // If connect, then create the COMPort
            port = new COMPort(std::move(R"(\\.\)" + portlist[selectedPort].first), baud);
        }
        if(portlist.empty()) ImGui::EndDisabled();
        if(disable) ImGui::EndDisabled();

        // If the port failed to allocate then return
        if(!port) {
            ImGui::PopID();
            ImGui::PopID();
            return nullptr;
        }

        // If the port allocated but did not open then show an error
        if(port->didPortOpenFail()) {
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1, 0, 0, 1));
            ImGui::Text("Error Connecting to Serial Port (%u)", port->lastError());
            ImGui::PopStyleColor();

            ImGui::SameLine();
            if(ImGui::Button("OK##comportchoosererror")) {
                delete port;
                port = nullptr;
            }
        }

        // While the port is readying, don't return it just yet and display a loading spinner
        else if(!port->isOpen()) {
            ImGui::SameLine();
            ImGui::Spinner("##comportchooserspinner", 8, 1, WModule::REBECCA_PURPLE);
        }

        else {
            ImGui::PopID();
            ImGui::PopID();
            return port;
        }

        ImGui::PopID();
        ImGui::PopID();
        return nullptr;
    }
} // namespace LRI::RCI
