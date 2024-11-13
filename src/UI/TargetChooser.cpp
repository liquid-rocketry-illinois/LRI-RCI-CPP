#include "UI/TargetChooser.h"

#include <Windows.h>
#include <SetupAPI.h>
#include <devguid.h>
#include <fstream>
#include <set>
#include <interfaces/VirtualPort.h>

#include "imgui.h"
#include "RCP_Host/RCP_Host.h"

#include "utils.h"
#include "RCP_Host_Impl.h"
#include "interfaces/COMPort.h"

namespace LRI::RCI {
    TargetChooser* TargetChooser::instance = nullptr;

    TargetChooser::TargetChooser() : BaseUI(), interf(nullptr), chooser(nullptr), pollingRate(5), targetpaths(),
                                     targetconfig(), chosenConfig(0), interfaceoptions(), chosenInterface(0) {
        BaseUI::showWindow();
        if(std::filesystem::exists("targets/")) {
            for(const auto& file : std::filesystem::directory_iterator("targets/")) {
                if(file.is_directory() || !file.path().string().ends_with(".json")) continue;
                targetpaths.push_back(file.path().string());
            }
        }

        interfaceoptions.emplace_back("Serial Port");
        interfaceoptions.emplace_back("Virtual Port");
        chooser = new COMPortChooser(this);
    }

    TargetChooser* const TargetChooser::getInstance() {
        if(instance == nullptr) instance = new TargetChooser();
        return instance;
    }

    void TargetChooser::render() {
        ImGui::SetNextWindowPos(ImVec2(50 * scaling_factor, 50 * scaling_factor), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(ImVec2(550 * scaling_factor, 200 * scaling_factor), ImGuiCond_FirstUseEver);
        if(ImGui::Begin("Target Settings", nullptr, ImGuiWindowFlags_NoResize)) {
            ImGui::Text("Target Connection Status: ");
            ImGui::SameLine();
            if(interf && interf->isOpen()) {
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0, 1, 0, 1));
                ImGui::Text("Open");
                ImGui::PopStyleColor();

                ImGui::Text("Current Target Config: %s", targetconfig["name"].get<std::string>().c_str());
                ImGui::Text("Current Interface: %s", interf->interfaceType().c_str());

                ImGui::NewLine();
                ImGui::Text("Polling Rate (polls/second): ");
                ImGui::SameLine();
                ImGui::SetNextItemWidth(75 * scaling_factor);
                ImGui::InputInt("#pollinginput", &pollingRate, 1, 5);

                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1, 0, 0, 1));
                ImGui::Text("WARNING: Higher Polling Rates increase frame render time!");
                ImGui::PopStyleColor();
                ImGui::Text("Latest Frame Time (ms): %f", ImGui::GetIO().DeltaTime);

                for(int i = 0; i < pollingRate && interf->hasData(); i++) {
                    RCP_poll();
                }

                if(ImGui::Button("Close Interface")) {
                    RCP_shutdown();
                    delete interf;
                    interf = nullptr;
                }
            }

            else {
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1, 0, 0, 1));
                ImGui::Text("Closed");
                ImGui::PopStyleColor();

                ImGui::Text("Choose Target Config: ");
                ImGui::SameLine();
                if(targetpaths.empty()) ImGui::Text("No Target configs available");
                else if(ImGui::BeginCombo("##targetchoosecombo", targetpaths[chosenConfig].c_str())) {
                    for(size_t i = 0; i < targetpaths.size(); i++) {
                        bool selected = i == chosenConfig;
                        if(ImGui::Selectable((targetpaths[i] + "##targetchooser").c_str(), &selected))
                            chosenConfig = i;
                        if(selected) ImGui::SetItemDefaultFocus();
                    }
                    ImGui::EndCombo();
                }

                ImGui::Text("Interface Type: ");
                ImGui::SameLine();
                if(interfaceoptions.empty()) ImGui::Text("No available interfaces");
                else if(ImGui::BeginCombo("##interfacechooser", interfaceoptions[chosenInterface].c_str())) {
                    for(size_t i = 0; i < interfaceoptions.size(); i++) {
                        bool selected = i == chosenInterface;
                        if(ImGui::Selectable((interfaceoptions[i] + "##interfacechooser").c_str(), &selected)) {
                            if(chosenInterface != i) {
                                delete chooser;
                                switch(i) {
                                case 0:
                                    chooser = new COMPortChooser(this);
                                    break;

                                case 1:
                                    chooser = new VirtualPortChooser();
                                    break;

                                default:
                                    chooser = nullptr;
                                }
                            }
                            chosenInterface = i;
                        }

                        if(selected) ImGui::SetItemDefaultFocus();
                    }

                    ImGui::EndCombo();
                }

                ImGui::NewLine();
                ImGui::Separator();

                if(chooser != nullptr) {
                    RCP_Interface* _interf = chooser->render();
                    if(_interf != nullptr) {
                        interf = _interf;
                        RCP_init(callbacks);
                        std::ifstream config(targetpaths[chosenConfig]);
                        targetconfig = nlohmann::json::parse(config);
                    }
                }
            }
        }

        ImGui::End();
    }

    const RCP_Interface* TargetChooser::getInterface() const {
        return interf;
    }

    void TargetChooser::hideWindow() {
    }

    void TargetChooser::showWindow() {
    }


    TargetChooser::COMPortChooser::COMPortChooser(TargetChooser* targetchooser) : InterfaceChooser(targetchooser),
        portlist(), selectedPort(0), error(false) {
        enumSerialDevs();
    }

    TargetChooser::InterfaceChooser::InterfaceChooser(TargetChooser* _targetchooser) : targetchooser(_targetchooser) {
    }


    bool TargetChooser::COMPortChooser::enumSerialDevs() {
        portlist.clear();
        HANDLE devs = SetupDiGetClassDevs((LPGUID)&GUID_DEVCLASS_PORTS, 0, 0, DIGCF_PRESENT);
        if(devs == INVALID_HANDLE_VALUE) return false;

        SP_DEVINFO_DATA data;
        data.cbSize = sizeof(SP_DEVINFO_DATA);
        char s[80];

        for(DWORD i = 0; SetupDiEnumDeviceInfo(devs, i, &data); i++) {
            HKEY hkey = SetupDiOpenDevRegKey(devs, &data, DICS_FLAG_GLOBAL, 0, DIREG_DEV, KEY_READ);
            if(hkey == INVALID_HANDLE_VALUE) {
                return false;
            }

            char comname[16] = {0};
            DWORD len = 16;

            RegQueryValueEx(hkey, "PortName", 0, 0, (LPBYTE)comname, &len);
            RegCloseKey(hkey);
            if(comname[0] != 'C') continue;

            SetupDiGetDeviceRegistryProperty(devs, &data, SPDRP_FRIENDLYNAME, nullptr, (PBYTE)s, sizeof(s), nullptr);

            portlist.push_back(std::string(comname) + ": " + std::string(s));
        }
        SetupDiDestroyDeviceInfoList(devs);
        return true;
    }

    RCP_Interface* TargetChooser::COMPortChooser::render() {
        ImGui::Text("Choose Serial Port: ");
        ImGui::SameLine();
        if(portlist.empty()) ImGui::Text("No Ports Detected");
        else if(ImGui::BeginCombo("##portselectcombo", portlist[selectedPort].c_str())) {
            for(size_t i = 0; i < portlist.size(); i++) {
                bool selected = i == selectedPort;
                if(ImGui::Selectable((portlist[i] + "##portselectcombo").c_str(), &selected))
                    selectedPort = i;
                if(selected) ImGui::SetItemDefaultFocus();
            }

            ImGui::EndCombo();
        }

        if(ImGui::Button("Refresh List")) {
            selectedPort = 0;
            enumSerialDevs();
        }

        COMPort* port = nullptr;

        if(portlist.empty()) ImGui::BeginDisabled();
        bool connectattempt = ImGui::Button("Connect");
        if(connectattempt) {
            port = new COMPort(
                portlist[selectedPort].substr(0, portlist[selectedPort].find_first_of(':')).c_str(),
                CBR_115200);
        }
        if(portlist.empty()) ImGui::EndDisabled();

        if(connectattempt && (port == nullptr || !port->isOpen())) error = true;
        if(error) {
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1, 0, 0, 1));
            ImGui::Text("Error Connecting to Serial Port");
            ImGui::PopStyleColor();
            delete port;
            port = nullptr;
        }

        return port;
    }

    TargetChooser::VirtualPortChooser::VirtualPortChooser() : InterfaceChooser(nullptr) {}

    RCP_Interface* TargetChooser::VirtualPortChooser::render() {
        if(ImGui::Button("Open Virtual Port")) return new VirtualPort();
        return nullptr;
    }

}
