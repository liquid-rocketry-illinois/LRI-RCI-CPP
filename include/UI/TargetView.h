#ifndef LRI_CONTROL_PANEL_TARGETVIEW_H
#define LRI_CONTROL_PANEL_TARGETVIEW_H

#include <filesystem>

#include "window.h"
#include "data/TargetConfig.h"

namespace LRI::RCI {
    class TargetView : public Windowlet {
        target::TargetConfig config;
        std::optional<std::string> loadError;
        bool close = false;

    public:
        TargetView(const std::filesystem::path& configPath);
        ~TargetView() override = default;

        void render() override;
        bool shouldClose() override;
        std::string contextString() override;
    };
}

#endif // LRI_CONTROL_PANEL_TARGETVIEW_H
