#include "UI/UIControl.h"

#include "UI/PacketBuilder.h"
#include "UI/Sidebar.h"
#include "UI/TargetConnect.h"
#include "UI/TopBar.h"
#include "UI/style.h"

namespace LRI::RCI {
    // Fonts
    ImFont* font_regular;
    ImFont* font_bold;
    ImFont* font_italic;

    // Scaling factor for hidpi screens
    float scaling_factor;

    void tooltipTextAndKeybind(const char* text, const char* bind) {
        if(ImGui::BeginItemTooltip()) {
            ImGui::Text("%s", text);
            ImGui::BeginDisabled();
            ImGui::SameLine();
            ImGui::Text("[%s]", bind);
            ImGui::EndDisabled();
            ImGui::EndTooltip();
        }
    }

    namespace {
        ImVec2 viewportSize;
        Box window;
        Box sidebar;
        Box main;
        Box topBar;

        void calcBoxes() {
            ImVec2 newSize = ImGui::GetMainViewport()->Size;
            if(newSize == viewportSize) return;
            viewportSize = newSize;
            window = {0, 0, viewportSize};
            sidebar = {window.tl(), window.b(), scale(SIDEBAR_WIDTH)};
            topBar = {0, sidebar.r(), scale(TOPBAR_WIDTH), window.r()};
            main = {topBar.b(), sidebar.r(), window.br()};
        }
    } // namespace

    namespace UIControl {
        void setup() {}

        void render() {
            calcBoxes();
            TopBar::render(topBar);
            switch(Sidebar::render(sidebar)) {
            case SideBarOptions::CONNECT:
                TargetConnect::render(main);
                break;

            case SideBarOptions::PACKETB:
                PKTB::render(main);
                break;

            default:
                break;
            }
        }

        void shutdown() {}
    } // namespace UIControl
} // namespace LRI::RCI
