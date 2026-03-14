#ifndef LRI_CONTROL_PANEL_STYLE_H
#define LRI_CONTROL_PANEL_STYLE_H

#include "imgui.h"

namespace LRI::RCI {
    constexpr ImVec4 U32ToImVec4(ImU32 color) {
        float s = 1.0f / 255.0f;
        return {static_cast<float>((color >> IM_COL32_R_SHIFT) & 0xFF) * s,
                static_cast<float>((color >> IM_COL32_G_SHIFT) & 0xFF) * s,
                static_cast<float>((color >> IM_COL32_B_SHIFT) & 0xFF) * s,
                static_cast<float>((color >> IM_COL32_A_SHIFT) & 0xFF) * s};
    }

#define COLOR_CONSTANT(name, value) static constexpr ImU32 name = value;\
                                    static constexpr ImVec4 name ## F = U32ToImVec4(name)

    COLOR_CONSTANT(EEE, 0x11223344);

    static constexpr float SIDEBAR_WIDTH = 40;
    static constexpr float TOPBAR_WIDTH = 40;

    // Colors stored as ABGR
    // Solid Base Colors
    COLOR_CONSTANT(YELLOW, 0xF000CDDB);
    COLOR_CONSTANT(GREEN, 0xFF00FF00);
    COLOR_CONSTANT(RED, 0xFF0000FF);
    COLOR_CONSTANT(WHITE, 0xFFFFFFFF);
    COLOR_CONSTANT(BACKGROUND, 0xFF000000);

    // Purples
    COLOR_CONSTANT(DPURPLE, 0xFFA72B43);
    COLOR_CONSTANT(PURPLE, 0xFFB83C54);
    COLOR_CONSTANT(LPURPLE, 0xFFC94D65);
    COLOR_CONSTANT(LLPURPLE, 0xFFDA5E76);

    // Transparents
    // COLOR_CONSTANT(TRANSPARENT, 0x00000000);
    COLOR_CONSTANT(COLOR_TRANSPARENT, 0x00000000);
    COLOR_CONSTANT(GRAY_SEMITRANSPARENT, 0x88444444);
    COLOR_CONSTANT(LGRAY_SEMITRANSPARENT, 0x88777777);

#undef COLOR_CONSTANT
} // namespace LRI::RCI

#endif // LRI_CONTROL_PANEL_STYLE_H
