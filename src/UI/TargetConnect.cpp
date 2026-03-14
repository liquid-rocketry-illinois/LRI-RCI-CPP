#include "../../include/UI/TargetConnect.h"

#include <filesystem>
#include <vector>
#include "fontawesome.h"

#include "interfaces/RCP_Interface.h"

#include "UI/style.h"
#include "hardware/HardwareControl.h"
#include "utils.h"
#include "UI/UIControl.h"
#include "interfaces/COMPort.h"
#include "improgress.h"

namespace LRI::RCI::TargetConnect {
    namespace SerialPort {
        namespace {
            COMPort* port = nullptr;
            std::vector<std::pair<std::string, std::string>> portList;
            size_t selectedPort = 0;
            int baud = 0;
            bool arduinoMode = true;
        }

        RCP_Interface* render() {
            ImGui::PushID("COMPortChooser");

            bool disable = port;
            if(disable) ImGui::BeginDisabled();

            // It has a dropdown for which device, as listed in enumSerial()
            ImGui::Text("Choose Serial Port: ");
            ImGui::SameLine();
            if(portList.empty()) ImGui::Text("No Ports Detected");
            else if(ImGui::BeginCombo("##portselectcombo", portList[selectedPort].second.c_str())) {
                for(size_t i = 0; i < portList.size(); i++) {
                    ImGui::PushID(static_cast<int>(i));
                    bool selected = i == selectedPort;
                    if(ImGui::Selectable((portList[i]).second.c_str(), &selected)) selectedPort = i;
                    if(selected) ImGui::SetItemDefaultFocus();
                    ImGui::PopID();
                }

                ImGui::EndCombo();
            }

            if(ImGui::Button("Refresh List")) {
                selectedPort = 0;
                COMPort::enumSerialDevs(portList);
            }

            ImGui::NewLine();

            // Input for baud rate
            ImGui::Text("Baud Rate: ");
            ImGui::SameLine();
            ImGui::SetNextItemWidth(scale(100));
            ImGui::InputInt("##comportchooserbaudinput", &baud);
            if(baud < 0) baud = 0;
            else if(baud > 500'000) baud = 500'000;

            ImGui::SameLine();
            ImGui::Text(" | Arduino Mode: ");
            ImGui::SameLine();
            ImGui::Checkbox("##arduinomode", &arduinoMode);

            if(portList.empty()) ImGui::BeginDisabled();
            if(ImGui::Button("Connect")) {
                // If connect, then create the COMPort
                port = new COMPort(portList[selectedPort].first, baud, arduinoMode);
            }
            if(portList.empty()) ImGui::EndDisabled();
            if(disable) ImGui::EndDisabled();

            RCP_Interface* ret = nullptr;

            if(port == nullptr) {
                ret = nullptr;
            }

            // If the port allocated but did not open then show an error
            else if(port->didPortOpenFail()) {
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1, 0, 0, 1));
                ImGui::Text("Error Connecting to Serial Port stage %lu code %lu)", port->lastError().code,
                            port->lastError().stage);
                ImGui::PopStyleColor();

                ImGui::SameLine();
                if(ImGui::Button("OK##comportchoosererror")) {
                    delete port;
                    port = nullptr;
                }

                ret = nullptr;
            }

            // While the port is readying, don't return it just yet and display a loading spinner
            else if(!port->isOpen()) {
                ImGui::SameLine();
                ImGui::Spinner("##comportchooserspinner", 8, 1, PURPLE);
                ret = nullptr;
            }

            else {
                ret = port;
                port = nullptr;
            }

            ImGui::PopID();
            return ret;
        }
    }

    namespace {
        std::vector<std::pair<std::filesystem::path, std::string>> jsons;
        size_t selectedJson = 0;

        void refreshJsons() {
            jsons.clear();

            const auto& targets = getRoamingFolder() / "targets";
            for(const auto& file : std::filesystem::directory_iterator(targets)) {
                if(file.is_directory() || !file.path().string().ends_with(".json")) continue;
                jsons.emplace_back(file.path(), file.path().filename().string());
            }
        }
    }

    RCP_Interface* render(const Box& region) {
        if(jsons.empty()) refreshJsons();

        ImGui::SetNextWindowPos(region.tl());
        ImGui::SetNextWindowSize(region.size());
        ImGui::Begin("##targetconnect", nullptr, WFLAGS);

        ImGui::Text("Choose Target Config: ");
        ImGui::SameLine();

        if(jsons.empty()) ImGui::Text("No available configs");
        else if(ImGui::BeginCombo("##targetconfigcombo", jsons[selectedJson].second.c_str())) {
            for(size_t i = 0; i < jsons.size(); i++) {
                bool selected = i == selectedJson;
                if(ImGui::Selectable(jsons[i].second.c_str(), &selected)) selectedJson = i;
                if(selected) ImGui::SetItemDefaultFocus();
            }

            ImGui::EndCombo();
        }

        ImGui::SameLine();
        if(ImGui::Button("" ICON_FA_ROTATE_LEFT "##refreshjsons")) {
            refreshJsons();
            selectedJson = 0;
        }

        ImGui::Text("Polling Rate: ");
        ImGui::SetNextItemWidth(scale(100));
        ImGui::SameLine();
        ImGui::InputInt("##hwctrlpollingrateinput", &HWCTRL::POLLS_PER_UPDATE, 1, 5);
        if(HWCTRL::POLLS_PER_UPDATE < 1) HWCTRL::POLLS_PER_UPDATE = 1;
        ImGui::Text("Packets processed in last frame: %d", HWCTRL::PACKETS_POLLED_IN_LAST_FRAME);

        RCP_Interface* interf = nullptr;

        ImGui::NewLine();
        if(ImGui::BeginTabBar("##connecttab")) {
            if(ImGui::BeginTabItem("Serial Port", nullptr)) {
                interf = SerialPort::render();
                ImGui::EndTabItem();
            }

            if(ImGui::BeginTabItem("TCP Socket", nullptr)) ImGui::EndTabItem();
            if(ImGui::BeginTabItem("Virtual Port", nullptr)) ImGui::EndTabItem();
            ImGui::EndTabBar();
        }

        ImGui::End();

        return interf;
    }
} // namespace LRI::RCI::TargetConnect
