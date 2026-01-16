#include "UI/Sidebar.h"

#include "imgui.h"

#include "UI/UIControl.h"

namespace LRI::RCI::Sidebar {
    static constexpr ImGuiWindowFlags WFLAGS = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoDocking;

    SideBarOptions render() {
        ImGui::SetNextWindowPos(getSidebarArea().tl());
        ImGui::SetNextWindowSize(getSidebarArea().size());
        ImGui::Begin("##sidebar", nullptr, WFLAGS);

        ImGui::End();
        return SideBarOptions::TARGET_VIEW;
    }
} // namespace LRI::RCI::Sidebar
