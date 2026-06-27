#include "util/system.h"

#include <Shlobj.h>
#include <print>

#define STB_IMAGE_IMPLEMENTATION 1
#include "util/stb_image.h"

namespace LRI::RCI {
    namespace {
        std::filesystem::path roaming;
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
} // namespace LRI::RCI
