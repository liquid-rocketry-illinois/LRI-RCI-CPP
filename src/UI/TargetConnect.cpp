#include "../../include/UI/TargetConnect.h"

#include <filesystem>
#include <vector>
#include "fontawesome.h"

#include "interfaces/RCP_Interface.h"

#include "UI/UIControl.h"
#include "UI/style.h"
#include "hardware/HardwareControl.h"
#include "improgress.h"
#include "interfaces/COMPort.h"
#include "interfaces/TCPSocket.h"
#include "utils.h"

namespace LRI::RCI::TargetConnect {
    namespace COMPortChooser {
        namespace {
            COMPort* port = nullptr;
            std::vector<std::pair<std::string, std::string>> portList;
            size_t selectedPort = 0;
            int baud = 0;
            bool arduinoMode = true;
        } // namespace

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
    } // namespace COMPortChooser

    namespace TCPSocketChooser {
        namespace {
            int tcpport = 5000;
            int address[4] = {192, 168, 254, 2};
            bool server = false;

            TCPSocket* port = nullptr;
        } // namespace

        RCP_Interface* render() {
            ImGui::PushID("TCPSocketChooser");

            bool disable = port;
            if(disable) ImGui::BeginDisabled();

            ImGui::PushStyleColor(ImGuiCol_Text, GREEN);
            ImGui::Text("Make sure to set the computer's static IP in control panel!");
            ImGui::PopStyleColor();

            ImGui::Text("Server Mode: ");
            ImGui::SameLine();
            ImGui::Checkbox("##servermode", &server);

            if(!server) {
                ImGui::Text("Server IP Address: ");
                ImGui::SetNextItemWidth(scale(200));
                ImGui::SameLine();
                ImGui::InputInt4("##serverip", address);
                for(int& i : address) {
                    if(i < 0) i = 0;
                    else if(i > 255) i = 255;
                }
            }

            // Port input
            ImGui::Text("Port: ");
            ImGui::SameLine();
            ImGui::SetNextItemWidth(scale(48));
            ImGui::InputInt("##portinput", &tcpport, 0);
            if(tcpport < 0) tcpport = 0;
            else if(tcpport > 65535) tcpport = 65535;

            // Once confirm is pushed, create the interface and begin waiting for a connection
            if(ImGui::Button(server ? "Begin Hosting" : "Connect")) {
                port = new TCPSocket(tcpport,
                                     server ? sf::IpAddress(0, 0, 0, 0)
                                            : sf::IpAddress(address[0], address[1], address[2], address[3]));
            }
            if(disable) ImGui::EndDisabled();

            RCP_Interface* ret = nullptr;

            if(port == nullptr) ret = nullptr;

            else if(port->didPortOpenFail()) {
                ImGui::PushStyleColor(ImGuiCol_Text, RED);
                TCPSocket::Error error = port->lastError();
                ImGui::Text("Error opening TCP socket: stage %lu, code %lu", error.stage, error.code);
                ImGui::PopStyleColor();

                ImGui::SameLine();
                if(ImGui::Button("OK")) {
                    delete port;
                    port = nullptr;
                }

                ret = nullptr;
            }

            else if(!port->isOpen()) {
                ImGui::SameLine();
                ImGui::Text("Waiting for connection");
                ImGui::SameLine();
                ImGui::Spinner("##tcpwaitspinner", 8, 1, WHITE);

                if(ImGui::Button("Cancel")) {
                    delete port;
                    port = nullptr;
                }
                ret = nullptr;
            }

            else {
                ret = port;
                port = nullptr;
            }

            ImGui::PopID();
            return ret;
        }
    } // namespace TCPSocketChooser

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
    } // namespace

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
            if(ImGui::BeginTabItem("Serial Port")) {
                interf = COMPortChooser::render();
                ImGui::EndTabItem();
            }

            if(ImGui::BeginTabItem("TCP Socket")) {
                interf = TCPSocketChooser::render();
                ImGui::EndTabItem();
            }
            if(ImGui::BeginTabItem("Virtual Port")) ImGui::EndTabItem();
            ImGui::EndTabBar();
        }

        ImGui::End();

        return interf;
    }
} // namespace LRI::RCI::TargetConnect
