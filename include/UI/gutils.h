#ifndef LRI_CONTROL_PANEL_GUTILS_H
#define LRI_CONTROL_PANEL_GUTILS_H

#include <string>
#include <Windows.h>

#include "imgui.h"
#include "glfw/glfw3.h"

namespace LRI::RCI {
    namespace style {
        extern float scaling_factor;

        void setup();
        void cleanup();
        GLFWwindow* getSharedResources();
        ImFontAtlas* getSharedFonts();
        GLuint birdIcon();

        void fontNormal(float size = 0);
        void fontBold(float size = 0);
        void fontItalic(float size = 0);

        void setWindowIcon(GLFWwindow* window);
        void resetFontFrameCount();

        const std::string& versionString();
        const ImVec2& verStringSize();
    }

    inline float scale(int val) { return val * style::scaling_factor; }
    inline float scale(float val) { return val * style::scaling_factor; }
    inline ImVec2 scale(const ImVec2& val) { return val * style::scaling_factor; }
    inline float operator""_sc(unsigned long long val) { return val * style::scaling_factor; }
    inline float operator""_sc(long double val) { return static_cast<float>(val) * style::scaling_factor; }

    constexpr ImVec2 V0 = ImVec2{0, 0};

    void pingFonts();
}

#endif // LRI_CONTROL_PANEL_GUTILS_H
