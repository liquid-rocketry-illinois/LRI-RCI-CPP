#include "UI/UIControl.h"

#include "UI/style.h"
#include "UI/Sidebar.h"

namespace LRI::RCI {
    // Fonts
    ImFont* font_regular;
    ImFont* font_bold;
    ImFont* font_italic;

    // Scaling factor for hidpi screens
    float scaling_factor;

    namespace {
        ImVec2 viewportSize;
        Box window;
        Box sidebar;
        Box main;

        void calcBoxes() {
            ImVec2 newSize = ImGui::GetMainViewport()->Size;
            if(newSize == viewportSize) return;
            viewportSize = newSize;
            window = {0, 0, viewportSize};
            sidebar = {window.tl(), window.b(), SIDEBAR_WIDTH};
            main = {sidebar.r(), 0, window.br()};
        }
    } // namespace

    const Box& getMainContentArea() { return main; }
    const Box& getSidebarArea() { return sidebar; }

    namespace UIControl {
        void setup() {}

        void render() {
            calcBoxes();
            Sidebar::render();
        }

        void shutdown() {}
    } // namespace UIControl
} // namespace LRI::RCI
