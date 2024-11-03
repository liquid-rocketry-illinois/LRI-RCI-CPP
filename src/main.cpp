#include <Windows.h>
#include "GLFW/glfw3.h"
#include "imgui.h"
#include "utils.h"
#include "devices/COMPort.h"
#include <setupapi.h>
#include <devguid.h>
#include <vector>

std::vector<std::string>* enumSerial();

int main() {
    if(!glfwInit()) {
        return -1;
    }

    glfwWindowHint(GLFW_MAXIMIZED, GLFW_TRUE);
    GLFWwindow* window = glfwCreateWindow(1920, 1080, "LRI RCI", nullptr, nullptr);
    if(!window) {
        return -1;
    }

    char portname[256] = {0};
    LRI::COMPort* port = nullptr;
    bool error = false;
    bool writedata = false;
    uint8_t buffer[64] = {0};

    std::vector<std::string>* devlist = nullptr;

    LRI::RCI::imgui_init(window);

    while(!glfwWindowShouldClose(window)) {
        LRI::RCI::imgui_prerender(window);



        LRI::RCI::imgui_postrender(window);
    }

    LRI::RCI::imgui_shutdown(window);

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

