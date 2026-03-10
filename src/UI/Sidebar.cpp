#include "UI/Sidebar.h"

#include <vector>

#include "fontawesome.h"
#include "imgui.h"

#include "UI/UIControl.h"
#include "UI/style.h"

namespace LRI::RCI::Sidebar {
    static constexpr ImGuiWindowFlags WFLAGS = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoDocking;

    namespace {
        SideBarOptions current = SideBarOptions::NONE;
        bool debugmode = false;

        struct SidebarButton {
            SideBarOptions option;
            const char* buttonText;
            const char* tooltipText;
            const char* keybindText;
            ImGuiKey keybind;
        };

        // clang-format off
        std::vector<SidebarButton> BUTTONS = {
            {
                .option = SideBarOptions::CONNECT,
                .buttonText = "" ICON_FA_SATELLITE_DISH,
                .tooltipText = "Target Connection",
                .keybindText = "T",
                .keybind = ImGuiKey_T
            },
            {
                .option = SideBarOptions::OVERVIEW,
                .buttonText = "" ICON_FA_ROCKET,
                .tooltipText = "Overview",
                .keybindText = "O",
                .keybind = ImGuiKey_O
            },
            {
                .option = SideBarOptions::PID,
                .buttonText = "" ICON_FA_DIAGRAM_PROJECT,
                .tooltipText = "P&ID",
                .keybindText = "P",
                .keybind = ImGuiKey_P
            },
            {
                .option = SideBarOptions::CONFIG,
                .buttonText = "" ICON_FA_PENCIL,
                .tooltipText = "Config Editor",
                .keybindText = "C",
                .keybind = ImGuiKey_C
            },
            {
                .option = SideBarOptions::HDF,
                .buttonText = "" ICON_FA_CHART_COLUMN,
                .tooltipText = "Log Editor",
                .keybindText = "L",
                .keybind = ImGuiKey_L
            }
        };

        std::vector<SidebarButton> DEBUG_BUTTONS = {
            {
                .option = SideBarOptions::PACKETB,
                .buttonText = "" ICON_FA_HAMMER,
                .tooltipText = "Packet Builder",
                .keybindText = "B",
                .keybind = ImGuiKey_B
            },
            {
                .option = SideBarOptions::PACKETI,
                .buttonText = "" ICON_FA_NETWORK_WIRED,
                .tooltipText = "Packet Inspector",
                .keybindText = "I",
                .keybind = ImGuiKey_I
            }
        };
        //clang-format on

        void renderButtons(const std::vector<SidebarButton>& buttons, const ImVec2& bsize) {
            for(const auto& btn : buttons) {
                bool recolor = current == btn.option;
                if(recolor) {
                    ImGui::PushStyleColor(ImGuiCol_Button, PURPLE);
                    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, LPURPLE);
                    ImGui::PushStyleColor(ImGuiCol_ButtonActive, LLPURPLE);
                }
                ImGui::SetCursorPosX(0);
                if(ImGui::Button(btn.buttonText, bsize) || ImGui::IsKeyPressed(btn.keybind, false)) {
                    if(current == btn.option) current = SideBarOptions::NONE;
                    else current = btn.option;
                }
                if(recolor) ImGui::PopStyleColor(3);
                tooltipTextAndKeybind(btn.tooltipText, btn.keybindText);
            }
        }
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

        renderButtons(BUTTONS, bsize);
        if(debugmode) renderButtons(DEBUG_BUTTONS, bsize);

        ImGui::PopStyleColor(3);
        ImGui::PopStyleVar();

        ImGui::End();
        return current;
    }
} // namespace LRI::RCI::Sidebar
