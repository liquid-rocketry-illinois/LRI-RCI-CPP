#ifndef LRI_CONTROL_PANEL_SYSTEM_H
#define LRI_CONTROL_PANEL_SYSTEM_H

#include <filesystem>
#include <future>
#include <vector>

namespace LRI::RCI {
    [[nodiscard]] const std::filesystem::path& roamingFolder();
    void detectRoamingFolder();

    void keepScreenAwake();
    void allowScreenSleep();

    void enumSerialDevs();
    const std::vector<std::pair<std::string, std::string>>& serialDevs();
    std::future<std::filesystem::path> pickFile(const std::string& filter = "",
                                                const std::string& defaultLocation = "");
} // namespace LRI::RCI

#endif // LRI_CONTROL_PANEL_SYSTEM_H
