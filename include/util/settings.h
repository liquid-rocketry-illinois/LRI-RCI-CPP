#ifndef LRI_CONTROL_PANEL_SETTINGS_H
#define LRI_CONTROL_PANEL_SETTINGS_H

#include <vector>
#include <filesystem>
#include <string>
#include <array>

namespace LRI::RCI::settings {
    enum class RecentType {
        TARGET,
        TLOG,
    };

    struct Recent {
        RecentType type = RecentType::TARGET;
        std::string displayName;
        std::array<char, 4> display_char;
        std::string connectName;
        std::filesystem::path path;
    };

    void loadUsersettings();
    bool hadParseError();
    const std::string& getErrorString();
    void loadFreshConfig();
    void writeSettings();

    const std::vector<Recent>& getRecents();
    void updateRecents(ptrdiff_t removeidx, Recent nrecent);
}

#endif // LRI_CONTROL_PANEL_SETTINGS_H
