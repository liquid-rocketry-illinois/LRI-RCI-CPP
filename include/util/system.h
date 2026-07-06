#ifndef LRI_CONTROL_PANEL_SYSTEM_H
#define LRI_CONTROL_PANEL_SYSTEM_H

#include <filesystem>
#include <vector>

namespace LRI::RCI {
    [[nodiscard]] const std::filesystem::path& roamingFolder();
    void detectRoamingFolder();

    void keepScreenAwake();
    void allowScreenSleep();

    void enumSerialDevs();
    const std::vector<std::pair<std::string, std::string>>& serialDevs();
}

#endif // LRI_CONTROL_PANEL_SYSTEM_H
