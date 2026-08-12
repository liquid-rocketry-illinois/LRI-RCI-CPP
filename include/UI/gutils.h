#ifndef LRI_CONTROL_PANEL_GUTILS_H
#define LRI_CONTROL_PANEL_GUTILS_H

#include "imgui.h"
#include "glfw/glfw3.h"

namespace LRI::RCI {
    namespace style {
        extern float scaling_factor;

        void setupResources();
        void cleanupResources();
        GLFWwindow* getSharedResources();
        ImFontAtlas* getSharedFonts();
        GLuint birdIcon();

        void fontNormal();
        void fontBold();
        void fontItalic();
    }

    inline float scale(int val) { return val * style::scaling_factor; }
    inline float scale(float val) { return val * style::scaling_factor; }
    inline ImVec2 scale(const ImVec2& val) { return val * style::scaling_factor; }
    inline float operator""_sc(unsigned long long val) { return val * style::scaling_factor; }
    inline float operator""_sc(long double val) { return static_cast<float>(val) * style::scaling_factor; }

    constexpr ImVec2 V0 = ImVec2{0, 0};
}

#endif // LRI_CONTROL_PANEL_GUTILS_H
