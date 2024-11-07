#include "UI/TargetChooser.h"

#include <Windows.h>
#include <SetupAPI.h>
#include <devguid.h>

#include "imgui.h"
#include "RCP_Host/RCP_Host.h"

#include "utils.h"
#include "RCP_Host_Impl.h"
#include "devices/COMPort.h"

namespace LRI::RCI {
    TargetChooser* TargetChooser::instance = nullptr;

    TargetChooser::TargetChooser() : BaseUI(), portlist(), selectedPort(0), port(nullptr) {
    }

    const TargetChooser* TargetChooser::getInstance() {
        if(instance == nullptr) instance = new TargetChooser();
        return instance;
    }

    void TargetChooser::render() {
        ImGui::SetNextWindowPos(ImVec2(50 * scaling_factor, 50 * scaling_factor), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(ImVec2(400 * scaling_factor, 200 * scaling_factor), ImGuiCond_FirstUseEver);
        if(ImGui::Begin("Choose COM Port", nullptr, ImGuiWindowFlags_NoResize)) {
            ImGui::Text("Current Port Status: ");
            ImGui::SameLine();
            if(port && port->isOpen()) {
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0, 1, 0, 1));
                ImGui::Text("Open");
                ImGui::PopStyleColor();

                if(ImGui::Button("Close Port")) {
                    delete port;
                    port = nullptr;
                    RCP_shutdown();
                }
            }

            else {
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1, 0, 0, 1));
                ImGui::Text("Closed");
                ImGui::PopStyleColor();
                ImGui::Text("Choose Serial Port: ");
                ImGui::SameLine();
                if(portlist.empty()) ImGui::Text("No Ports Detected");
                else if(ImGui::BeginCombo("##portselectcombo", portlist[selectedPort].c_str())) {
                    for(size_t i = 0; i < portlist.size(); i++) {
                        bool selected = i == selectedPort;
                        if(ImGui::Selectable((portlist[selectedPort] + "##portselectcombo").c_str(), &selected))
                            selectedPort = i;
                        if(selected) ImGui::SetItemDefaultFocus();
                    }
                }

                if(ImGui::Button("Refresh List")) {
                    selectedPort = 0;
                    enumDevs();
                }

                if(portlist.empty()) ImGui::BeginDisabled();
                if(ImGui::Button("Connect")) {
                    port = new COMPort(
                        portlist[selectedPort].substr(0, portlist[selectedPort].find_first_of(':')).c_str(),
                        CBR_115200);

                    RCP_init(callbacks);
                }

                if(portlist.empty()) ImGui::EndDisabled();
            }
        }

        ImGui::End();
    }


    bool TargetChooser::enumDevs() {
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

            portlist.push_back(std::string(comname) + ":" + std::string(s));
        }
        SetupDiDestroyDeviceInfoList(devs);
        return true;
    }

    const RCP_Interface* TargetChooser::getInterface() const {
        return port;
    }

    void TargetChooser::destroy() {}


}
