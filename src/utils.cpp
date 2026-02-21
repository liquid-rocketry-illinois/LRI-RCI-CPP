#include "utils.h"

#include <fstream>
#include <shlobj_core.h>

#include "RCP_Host/RCP_Host.h"
#include "imgui.h"

// A mish-mash of various different things that are useful
namespace LRI::RCI {
    // Fonts
    ImFont* font_regular;
    ImFont* font_bold;
    ImFont* font_italic;

    // Scaling factor for hidpi screens
    float scaling_factor;

    std::string devclassToString(RCP_DeviceClass devclass) {
        switch(devclass) {
        case RCP_DEVCLASS_TEST_STATE:
            return "Test State (Virtual Device)";

        case RCP_DEVCLASS_SIMPLE_ACTUATOR:
            return "Simple Actuator";

        case RCP_DEVCLASS_STEPPER:
            return "Stepper Motor";

        case RCP_DEVCLASS_CUSTOM:
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
