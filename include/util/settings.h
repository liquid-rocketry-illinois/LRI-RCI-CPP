#ifndef LRI_CONTROL_PANEL_SETTINGS_H
#define LRI_CONTROL_PANEL_SETTINGS_H

#include <vector>
#include <filesystem>
#include <string>

namespace LRI::RCI::settings {
    enum class RecentType {
        TARGET,
        TLOG,
    };

    struct Recent {
        RecentType type;
        std::string displayName;
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
