#ifndef LRI_CONTROL_PANEL_SYSTEM_H
#define LRI_CONTROL_PANEL_SYSTEM_H

#include <filesystem>

namespace LRI::RCI {
    [[nodiscard]] const std::filesystem::path& roamingFolder();
    void detectRoamingFolder();

    void keepScreenAwake();
    void allowScreenSleep();
}

#endif // LRI_CONTROL_PANEL_SYSTEM_H
