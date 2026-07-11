#include "UI/TargetView.h"
#include "imgui.h"

namespace LRI::RCI {
    TargetView::TargetView(const std::filesystem::path& configPath) :
        loadError(target::deserializeConfig(configPath, config)) {}

    void TargetView::render() {
        ImGui::Text("Hello: %s", config.name.c_str());
        if(ImGui::Button("Close")) close = true;
    }

    bool TargetView::shouldClose() {
        return close;
    }
} // namespace LRI::RCI
