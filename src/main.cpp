#include "GLFW/glfw3.h"
#include "imgui.h"
#include "setup.h"
#include "devices/COMPort.h"
#include <Windows.h>
#include <setupapi.h>
#include <devguid.h>
#include <vector>

#include <windowsx.h>
#include <cfgmgr32.h>

std::vector<std::string>* enumSerial();

int main() {


    if(!glfwInit()) {
        return -1;
    }

    glfwWindowHint(GLFW_MAXIMIZED, GLFW_TRUE);
    GLFWwindow* window = glfwCreateWindow(1920, 1080, "LRI RCP", nullptr, nullptr);
    if(!window) {
        return -1;
    }

    char portname[256] = {0};
    LRI::COMPort* port = nullptr;
    bool error = false;
    bool writedata = false;
    uint8_t buffer[64] = {0};

    std::vector<std::string>* devlist = nullptr;

    LRI::imgui_init(window);

    while(!glfwWindowShouldClose(window)) {
        LRI::imgui_prerender(window);

        ImGui::SetNextWindowPos(ImVec2(50 * LRI::scaling_factor, 50 * LRI::scaling_factor), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(ImVec2(350 * LRI::scaling_factor, 200 * LRI::scaling_factor), ImGuiCond_FirstUseEver);
        if(ImGui::Begin("Window Diagnostic", nullptr, LRI::window_flags)) {
            ImGui::Text("LRI Rocket Control Panel");
            int h, w;
            glfwGetFramebufferSize(window, &w, &h);
            ImGui::Text("Framebuffer size: %d x %d", h, w);
            glfwGetWindowSize(window, &w, &h);
            ImGui::Text("Window size: %d x %d", h, w);
            float xs, ys;
            glfwGetWindowContentScale(window, &xs, &ys);
            ImGui::Text("Window scale: x: %f, y: %f", xs, ys);

            ImGui::Text("Frame time: %f", ImGui::GetIO().DeltaTime);
            ImGui::Text("Frame rate: %f", ImGui::GetIO().Framerate);
        }

        ImGui::End();

        ImGui::SetNextWindowPos(ImVec2(700 * LRI::scaling_factor, 50 * LRI::scaling_factor), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(ImVec2(400 * LRI::scaling_factor, 400 * LRI::scaling_factor), ImGuiCond_FirstUseEver);
        if(ImGui::Begin("Rocket Control", nullptr, LRI::window_flags)) {
            ImGui::Text("COM Port name: ");
            ImGui::SameLine();
            ImGui::InputText("##comportinput", portname, 256);

            if(ImGui::Button(port == nullptr ? "Open" : "Close")) {
                if(port == nullptr) {
                    port = new LRI::COMPort(portname);
                    if(!port->isOpen()) error = true;
                }

                else {
                    delete port;
                    port = nullptr;
                    error = false;
                }
            }

            if(port == nullptr) ImGui::BeginDisabled();

            if(ImGui::Button(writedata ? "Stop Writing" : "Start Writing")) {
                writedata = !writedata;
            }

            if(port != nullptr && writedata && port->write(buffer, 64) < 0) {
                error = true;
            }

            if(port == nullptr) ImGui::EndDisabled();

            if(error) {
                ImGui::Text("Error: %d", GetLastError());
            }
        }

        ImGui::End();

        if(ImGui::Begin("COM Port Enumerator", nullptr, LRI::window_flags)) {
            if(ImGui::Button("Enumerate")) {
                devlist = enumSerial();
            }

            if(devlist == nullptr) ImGui::Text("devlist null");
            else {
                for(int i = 0; i < devlist->size(); i++) {
                    ImGui::Text("%s", devlist->at(i).c_str());
                }
            }

        }

        ImGui::End();

        LRI::imgui_postrender(window);
    }

    LRI::imgui_shutdown(window);

    return 0;
}


std::vector<std::string>* enumSerial() {
    HANDLE devs = SetupDiGetClassDevs((LPGUID) &GUID_DEVCLASS_PORTS, 0, 0, DIGCF_PRESENT);
    if(devs == INVALID_HANDLE_VALUE) return nullptr;

    SP_DEVINFO_DATA data;
    data.cbSize = sizeof(SP_DEVINFO_DATA);
    char s[80];

    std::vector<std::string>* devlist = new std::vector<std::string>();


    for(DWORD i = 0; SetupDiEnumDeviceInfo(devs, i, &data); i++) {
        HKEY hkey = SetupDiOpenDevRegKey(devs, &data, DICS_FLAG_GLOBAL, 0, DIREG_DEV, KEY_READ);
        if(hkey == INVALID_HANDLE_VALUE) {
            delete devlist;
            return nullptr;
        }

        char comname[16] = {0};
        DWORD len = 16;

        RegQueryValueEx(hkey, "PortName", 0, 0, (LPBYTE) comname, &len);
        RegCloseKey(hkey);
        if(comname[0] != 'C') continue;

        SetupDiGetDeviceRegistryProperty(devs, &data, SPDRP_FRIENDLYNAME, nullptr, (PBYTE) s, sizeof(s), nullptr);

        devlist->push_back(std::string(s) + std::string(comname));

    }
    SetupDiDestroyDeviceInfoList(devs);
    return devlist;
}

