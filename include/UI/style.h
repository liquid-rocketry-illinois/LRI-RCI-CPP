#ifndef LRI_CONTROL_PANEL_STYLE_H
#define LRI_CONTROL_PANEL_STYLE_H

#include "imgui.h"

namespace LRI::RCI {
    static constexpr float SIDEBAR_WIDTH = 75;

    static constexpr ImU32 GREEN = 0xFF00FF00; // Colors are stored as ABGR
    static constexpr ImU32 YELLOW = 0xF000CDDB;
    static constexpr ImU32 RED = 0xFF0000FF;
    static constexpr ImU32 PURPLE = 0xFF993366;
    static constexpr ImU32 WHITE = 0xFFFFFFFF;
    static constexpr ImVec4 BACKGROUND_COLOR{0.f, 0.f, 0.f, 0.f};

}

#endif // LRI_CONTROL_PANEL_STYLE_H
