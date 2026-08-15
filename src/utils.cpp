#include "utils.h"

#include <fstream>
#include <shlobj_core.h>
#include <SetupAPI.h>
#include <devguid.h>

#include "RCP_Host/RCP_Host.h"
#include "imgui.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

// A mish-mash of various different things that are useful
namespace LRI::RCI {
    // Fonts
    ImFont* font_regular;
    ImFont* font_bold;
    ImFont* font_italic;

    std::string devclassToString(RCP_DeviceClass devclass) {
        switch(devclass) {
        case RCP_DEVCLASS_TEST_STATE:
            return "Test State (Virtual Device)";

        case RCP_DEVCLASS_SIMPLE_ACTUATOR:
            return "Simple Actuator";

        case RCP_DEVCLASS_STEPPER:
            return "Stepper Motor";

        case RCP_DEVCLASS_TARGET_LOG:
            return "Raw Data (Virtual Device)";

        case RCP_DEVCLASS_AM_PRESSURE:
            return "Ambient Pressure";

        case RCP_DEVCLASS_TEMPERATURE:
            return "Ambient Temperature";

        case RCP_DEVCLASS_PRESSURE_TRANSDUCER:
            return "Pressure Transducer";

        case RCP_DEVCLASS_RELATIVE_HYGROMETER:
            return "Relative Hygrometer";

        case RCP_DEVCLASS_LOAD_CELL:
            return "Load Cell (weight)";

        case RCP_DEVCLASS_POWERMON:
            return "Power Monitor";

        case RCP_DEVCLASS_ACCELEROMETER:
            return "Accelerometer";

        case RCP_DEVCLASS_GYROSCOPE:
            return "Gyroscope";

        case RCP_DEVCLASS_MAGNETOMETER:
            return "Magnetometer";

        case RCP_DEVCLASS_GPS:
            return "GPS";

        default:
            return "Unknown";
        }
    }

    static std::filesystem::path roamingFolder;

    const std::filesystem::path& getRoamingFolder() { return roamingFolder; }

    void detectRoamingFolder() {
#ifdef RCIDEBUG
        char buf[256];
        DWORD retlen = GetModuleFileName(nullptr, buf, sizeof(buf));
        if(retlen >= sizeof(buf)) std::exit(-1);
        roamingFolder = buf;
        roamingFolder = roamingFolder.parent_path() / "roaming";
#else
        PWSTR pathstr = nullptr;
        HRESULT res = SHGetKnownFolderPath(FOLDERID_RoamingAppData, 0, nullptr, &pathstr);
        if(res != S_OK || !pathstr) std::exit(-1);
        roamingFolder = pathstr;
        roamingFolder /= "LRI Electronics";
        roamingFolder /= "Rocket Control Interface (RCI)";
        CoTaskMemFree(pathstr);
#endif

        if(std::filesystem::exists(roamingFolder)) {
            if(!std::filesystem::is_directory(roamingFolder)) {
                std::exit(-1);
            }
        }

        else std::filesystem::create_directories(roamingFolder);

        auto targetsFolder = roamingFolder / "targets";
        if(std::filesystem::exists(targetsFolder)) {
            if(!std::filesystem::is_directory(targetsFolder)) std::exit(-1);
        }

        else {
            std::filesystem::copy("targets", targetsFolder);
            std::ofstream readme(targetsFolder / "README");
            readme << "This folder stores user-configured UI layouts.\nNote that the target jsons here are only for "
                      "reference, the versions actually used are in the executable's folder"
                   << std::endl;
        }
    }

    void preventScreenTurnoff() {
#ifdef _WIN32
        SetThreadExecutionState(ES_DISPLAY_REQUIRED | ES_CONTINUOUS);
#else
#error "Linux not yet supported"
#endif
    }

    void allowScreenTurnoff() {
#ifdef _WIN32
        SetThreadExecutionState(ES_CONTINUOUS);
#else
#error "Linux not yet supported"
#endif
    }

    static std::vector<std::pair<std::string, std::string>> serialDevs;

    // Honestly I dont know what this does its some Windows spaghetti I stole from SO but it works so yay
    // https://stackoverflow.com/a/77752863
    void enumSerialDevs() {
        serialDevs.clear();
        HANDLE devs = SetupDiGetClassDevs(&GUID_DEVCLASS_PORTS, nullptr, nullptr, DIGCF_PRESENT);
        if(devs == INVALID_HANDLE_VALUE) return;

        SP_DEVINFO_DATA data;
        data.cbSize = sizeof(SP_DEVINFO_DATA);
        char s[80];

        for(DWORD i = 0; SetupDiEnumDeviceInfo(devs, i, &data); i++) {
            HKEY hkey = SetupDiOpenDevRegKey(devs, &data, DICS_FLAG_GLOBAL, 0, DIREG_DEV, KEY_READ);
            if(hkey == INVALID_HANDLE_VALUE) {
                return;
            }

            char comname[16];
            DWORD len = 16;

            RegQueryValueEx(hkey, "PortName", nullptr, nullptr, (LPBYTE) comname, &len);
            RegCloseKey(hkey);
            if(comname[0] != 'C') continue;

            SetupDiGetDeviceRegistryProperty(devs, &data, SPDRP_FRIENDLYNAME, nullptr, (PBYTE) s, sizeof(s), nullptr);

            // Somehow we end up with the name we need to open the port, and a more user friendly display string.
            // These get appended to this vector for later
            serialDevs.emplace_back(std::make_pair(std::string(comname), std::string(comname) + " : " + std::string(s)));
        }

        SetupDiDestroyDeviceInfoList(devs);
    }

    const std::vector<std::pair<std::string, std::string>>& getSerialDevs() { return serialDevs; }

} // namespace LRI::RCI

namespace ImGui {
    // See utils.h
    bool TimedButton(const char* label, LRI::RCI::StopWatch& sw, const ImVec2& size) {
        Button(label, size);
        if(IsItemActivated()) sw.reset();
        return IsItemActive();
    }

    TimedButton::TimedButton(const char* label) : label(label), clicked(false) {}

    bool TimedButton::render() {
        Button(label);
        clicked = IsItemActive();
        if(IsItemActivated()) timer.reset();
        return clicked;
    }

    float TimedButton::getHoldTime() const { return clicked ? timer.timeSince() : 0; }
} // namespace ImGui
