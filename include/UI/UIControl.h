#ifndef LRI_CONTROL_PANEL_UICONTROL_H
#define LRI_CONTROL_PANEL_UICONTROL_H

#include "imgui.h"

#include "positioning.h"

namespace LRI::RCI {
    extern ImFont* font_regular;
    extern ImFont* font_bold;
    extern ImFont* font_italic;
    extern float scaling_factor;

    inline float scale(float val) { return scaling_factor * val; }
    inline ImVec2 scale(ImVec2 val) { return val * scaling_factor; }

    const Box& getSidebarArea();
    const Box& getMainContentArea();

    namespace UIControl {
        void setup();
        void render();
        void shutdown();
    }
}

#endif // LRI_CONTROL_PANEL_UICONTROL_H
