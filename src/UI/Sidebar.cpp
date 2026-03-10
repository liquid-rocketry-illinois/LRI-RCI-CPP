#include "UI/Sidebar.h"

#include "fontawesome.h"
#include "imgui.h"

#include "UI/style.h"
#include "UI/UIControl.h"

namespace LRI::RCI::Sidebar {
    static constexpr ImGuiWindowFlags WFLAGS = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoDocking;

    namespace {
        SideBarOptions current = SideBarOptions::NONE;
        bool debugmode = false;
    }

    SideBarOptions render() {
        if(ImGui::IsKeyChordPressed(ImGuiKey_ModCtrl | ImGuiKey_D)) debugmode = !debugmode;

        ImGui::SetNextWindowPos(getSidebarArea().tl());
        ImGui::SetNextWindowSize(getSidebarArea().size());
        ImGui::Begin("##sidebar", nullptr, WFLAGS);

        ImVec2 bsize = {getSidebarArea().width(), getSidebarArea().width()};

        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, scale(20));
        ImGui::PushStyleColor(ImGuiCol_Button, TTRANSPARENT);
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, GRAY_SEMITRANSPARENT);
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, LGRAY_SEMITRANSPARENT);

        // Connection
        ImGui::SetCursorPosX(0);
        if(ImGui::Button("" ICON_FA_SATELLITE_DISH, bsize) || ImGui::IsKeyPressed(ImGuiKey_T, false)) {
            if(current == SideBarOptions::CONNECT) current = SideBarOptions::NONE;
            else current = SideBarOptions::CONNECT;
        }
        tooltipTextAndKeybind("Target Connection", "T");

        // ImGui::SetItemTooltip("[T] Target Connection");

        // Overview
        ImGui::SetCursorPosX(0);
        if(ImGui::Button("" ICON_FA_ROCKET, bsize) || ImGui::IsKeyPressed(ImGuiKey_O, false)) {
            if(current == SideBarOptions::OVERVIEW) current = SideBarOptions::NONE;
            else current = SideBarOptions::OVERVIEW;
        }
        tooltipTextAndKeybind("Overview", "O");

        // P&ID
        ImGui::SetCursorPosX(0);
        if(ImGui::Button("" ICON_FA_DIAGRAM_PROJECT, bsize) || ImGui::IsKeyPressed(ImGuiKey_P, false)) {
            if(current == SideBarOptions::PID) current = SideBarOptions::NONE;
            else current = SideBarOptions::PID;
        }
        tooltipTextAndKeybind("P&ID", "P");

        // config editor
        ImGui::SetCursorPosX(0);
        if(ImGui::Button("" ICON_FA_PENCIL, bsize) || ImGui::IsKeyPressed(ImGuiKey_C, false)) {
            if(current == SideBarOptions::CONFIG) current = SideBarOptions::NONE;
            else current = SideBarOptions::CONFIG;
        }
        tooltipTextAndKeybind("Config Editor", "C");

        // HDF viewer/editor
        ImGui::SetCursorPosX(0);
        if(ImGui::Button("" ICON_FA_CHART_COLUMN, bsize) || ImGui::IsKeyPressed(ImGuiKey_L, false)) {
            if(current == SideBarOptions::HDF) current = SideBarOptions::NONE;
            else current = SideBarOptions::HDF;
        }
        tooltipTextAndKeybind("Log Viewer", "L");

        if(!debugmode) {
            ImGui::PopStyleColor(3);
            ImGui::PopStyleVar();
            ImGui::End();
            return current;
        }

        // Packet builder
        ImGui::SetCursorPosX(0);
        if(ImGui::Button("" ICON_FA_HAMMER, bsize) || ImGui::IsKeyPressed(ImGuiKey_B, false)) {
            if(current == SideBarOptions::PACKETB) current = SideBarOptions::NONE;
            else current = SideBarOptions::PACKETB;
        }
        tooltipTextAndKeybind("Debug: Packet Builder", "B");

        // Packet inspector
        ImGui::SetCursorPosX(0);
        if(ImGui::Button("" ICON_FA_NETWORK_WIRED, bsize) || ImGui::IsKeyPressed(ImGuiKey_I, false)) {
            if(current == SideBarOptions::PACKETI) current = SideBarOptions::NONE;
            else current = SideBarOptions::PACKETI;
        }
        tooltipTextAndKeybind("Debug: Packet Inspector", "I");

        ImGui::PopStyleColor(3);
        ImGui::PopStyleVar();

        ImGui::End();
        return current;
    }
} // namespace LRI::RCI::Sidebar
