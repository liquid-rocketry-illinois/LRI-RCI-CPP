#ifndef LRI_CONTROL_PANEL_TARGETCONFIG_H
#define LRI_CONTROL_PANEL_TARGETCONFIG_H

#include <filesystem>
#include <string>
#include <set>

#include "HardwareQualifier.h"

namespace LRI::RCI::target {
    struct IDNamePair {
        uint8_t id;
        std::string name;
    };

    struct TargetConfig {
        std::string name;
        std::vector<IDNamePair> tests;
        std::set<HardwareQualifier> quals;
    };

    void serializeConfig(const std::filesystem::path& path, const TargetConfig& config);
    std::optional<std::string> deserializeConfig(const std::filesystem::path& path, TargetConfig& config);
}

#endif // LRI_CONTROL_PANEL_TARGETCONFIG_H
