#ifndef LRI_CONTROL_PANEL_JSON_H
#define LRI_CONTROL_PANEL_JSON_H

#include <filesystem>
#include <string>
#include <vector>

#include "RCP_Host/RCP_Host.h"

namespace LRI::RCI {
    struct TargetTest {
        uint8_t id;
        std::string name;
    };

    struct TargetDevice {
        RCP_DeviceClass devclass;
        std::vector<uint8_t> ids;
        std::vector<std::string> names;
    };

    struct TargetTable6 {
        bool refresh;
        std::vector<uint8_t> ids;
    };

    enum class SensorViewerMode {
        INVALID,
        CLASSIC,
        ABRIDGED,
        MULTI,
    };

    struct TargetTable7 {
        SensorViewerMode mode;
        bool classicShowControls;

        struct SensorViewerIDData {
            RCP_DeviceClass devclass;
            std::vector<uint8_t> ids;
            std::vector<uint8_t> channels;
            std::string multiTitle;
        };

        std::vector<SensorViewerIDData> ids;
    };

    struct TargetWModule {
        int type = 0;
        TargetTable6 t6data;
        TargetTable7 t7data;
    };

    struct TargetWindowlet {
        std::string title;
        std::vector<TargetWModule> modules;
    };

    struct TargetConfig {
        std::string name;
        std::vector<TargetTest> tests;
        std::vector<TargetDevice> devices;
        std::vector<TargetWindowlet> windows;
    };

    TargetConfig readConfig(const std::filesystem::path& path);
}

#endif // LRI_CONTROL_PANEL_JSON_H
