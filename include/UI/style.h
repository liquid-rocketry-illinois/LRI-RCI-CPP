#ifndef LRI_CONTROL_PANEL_STYLE_H
#define LRI_CONTROL_PANEL_STYLE_H

#include "imgui.h"

namespace LRI::RCI {
    static constexpr float SIDEBAR_WIDTH = 75;

    static constexpr ImU32 GREEN = 0xFF00FF00; // Colors are stored as ABGR
    static constexpr ImU32 YELLOW = 0xF000CDDB;
    static constexpr ImU32 RED = 0xFF0000FF;
    static constexpr ImU32 PURPLE = 0xFFB83C54; // 432ba7
    static constexpr ImU32 LPURPLE = 0xFFC94D65;
    static constexpr ImU32 LLPURPLE = 0xFFDA5E76;
    static constexpr ImU32 WHITE = 0xFFFFFFFF;
    static constexpr ImVec4 BACKGROUND_COLOR{0.f, 0.f, 0.f, 0.f};
    static constexpr ImU32 TTRANSPARENT = 0x00000000;
    static constexpr ImU32 GRAY_SEMITRANSPARENT = 0x88444444;
    static constexpr ImU32 LGRAY_SEMITRANSPARENT = 0x88777777;


}

#endif // LRI_CONTROL_PANEL_STYLE_H
