#include "UI/TopBar.h"

namespace LRI::RCI::TopBar {
    namespace {
        constexpr ImGuiWindowFlags WFLAGS = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoDocking;
    }
    void render(const Box& region) {
        ImGui::SetNextWindowPos(region.tl());
        ImGui::SetNextWindowSize(region.size());
        ImGui::Begin("##topbar", nullptr, WFLAGS);
        ImGui::Text("Target Not Connected");
        ImGui::End();
    }
} // namespace LRI::RCI::TopBar
