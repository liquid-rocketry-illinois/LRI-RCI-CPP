#include "util/system.h"

#include <Shlobj.h>
#include <SetupAPI.h>
#include <devguid.h>
#include <print>

#define STB_IMAGE_IMPLEMENTATION 1
#include "util/stb_image.h"

namespace LRI::RCI {
    namespace {
        std::filesystem::path roaming;
        std::vector<std::pair<std::string, std::string>> serials;
    }

    const std::filesystem::path& roamingFolder() { return roaming; }

    void detectRoamingFolder() {
#ifdef RCIDEBUG
        char buf[256];
        DWORD retlen = GetModuleFileName(nullptr, buf, sizeof(buf));
        if(retlen >= sizeof(buf)) {
            std::println("Executable path too long for roaming detection: {}", buf);
            std::exit(-1);
        }

        roaming = buf;
        roaming = roaming.parent_path() / "roaming";
#else
        PWSTR pathstr = nullptr;
        HRESULT res = SHGetKnownFolderPath(FOLDERID_RoamingAppData, 0, nullptr, &pathstr);
        if(res != S_OK || !pathstr) std::exit(-1);
        roaming = pathstr;
        roaming /= "LRI Electronics";
        roaming /= "Rocket Control Interface (RCI)";
        CoTaskMemFree(pathstr);
#endif

        if(std::filesystem::exists(roaming)) {
            if(!std::filesystem::is_directory(roaming)) {
                std::println("{} exists but is a file", roaming.string());
                std::exit(-1);
            }
        }

        else std::filesystem::create_directories(roaming);

        auto targetsFolder = roaming / "targets";
        if(std::filesystem::exists(targetsFolder)) {
            if(!std::filesystem::is_directory(targetsFolder)) {
                std::println("{} exists but is a file", targetsFolder.string());
                std::exit(-1);
            }
        }

        else std::filesystem::create_directories(targetsFolder);
    }

    void keepScreenAwake() { SetThreadExecutionState(ES_DISPLAY_REQUIRED | ES_CONTINUOUS); }
    void allowScreenSleep() { SetThreadExecutionState(ES_CONTINUOUS); }

    // Honestly I dont know what this does its some Windows spaghetti I stole from SO but it works so yay
    // https://stackoverflow.com/a/77752863
    void enumSerialDevs() {
        serials.clear();
        HANDLE devs = SetupDiGetClassDevs(&GUID_DEVCLASS_PORTS, nullptr, nullptr, DIGCF_PRESENT);
        if(devs == INVALID_HANDLE_VALUE) return;

        SP_DEVINFO_DATA data;
        data.cbSize = sizeof(SP_DEVINFO_DATA);
        char s[80];

        for(DWORD i = 0; SetupDiEnumDeviceInfo(devs, i, &data); i++) {
            HKEY hkey = SetupDiOpenDevRegKey(devs, &data, DICS_FLAG_GLOBAL, 0, DIREG_DEV, KEY_READ);
            if(hkey == INVALID_HANDLE_VALUE) {
                continue;
            }

            char comname[16];
            DWORD len = 16;

            RegQueryValueEx(hkey, "PortName", nullptr, nullptr, (LPBYTE) comname, &len);
            RegCloseKey(hkey);
            if(comname[0] != 'C') continue;

            SetupDiGetDeviceRegistryProperty(devs, &data, SPDRP_FRIENDLYNAME, nullptr, (PBYTE) s, sizeof(s), nullptr);

            // Somehow we end up with the name we need to open the port, and a more user friendly display string.
            // These get appended to this vector for later
            serials.emplace_back(std::string(comname), std::string(comname) + " : " + std::string(s));
        }

        SetupDiDestroyDeviceInfoList(devs);
    }

    const std::vector<std::pair<std::string, std::string>>& serialDevs() {
        return serials;
    }
} // namespace LRI::RCI
