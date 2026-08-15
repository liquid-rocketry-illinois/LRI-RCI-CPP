#ifndef LRI_CONTROL_PANEL_GUTILS_H
#define LRI_CONTROL_PANEL_GUTILS_H

#include <Windows.h>
#include <functional>
#include <string>

#include "glfw/glfw3.h"
#include "imgui.h"

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

        void setColors();
    } // namespace style

    namespace colors {
        constexpr ImVec4 U32ToImVec4(ImU32 color) {
            constexpr float s = 1.0f / 255.0f;
            return {static_cast<float>((color >> IM_COL32_R_SHIFT) & 0xFF) * s,
                    static_cast<float>((color >> IM_COL32_G_SHIFT) & 0xFF) * s,
                    static_cast<float>((color >> IM_COL32_B_SHIFT) & 0xFF) * s,
                    static_cast<float>((color >> IM_COL32_A_SHIFT) & 0xFF) * s};
        }

#define COLOR_CONSTANT(name, value)                                                                                    \
    constexpr ImU32 name##i = value;                                                                                   \
    constexpr ImVec4 name = U32ToImVec4(name##i)

        COLOR_CONSTANT(CTRANSPARENT, 0x00000000);
        COLOR_CONSTANT(LOW_SEMITRANSPARENT, 0x0CFFFFFF);
        COLOR_CONSTANT(HIGH_SEMITRANSPARENT, 0x12FFFFFF);

        COLOR_CONSTANT(WINDOW_BG, 0xFF222222);
        COLOR_CONSTANT(TEXT, 0xFFFFFFFF);
        COLOR_CONSTANT(BUTTON_CLOSE_HOVERED, 0xFF0000EE);
        COLOR_CONSTANT(BUTTON_CLOSE_ACTIVE, 0xFF0000FF);

        COLOR_CONSTANT(REBECCA, 0xFF993366);

        COLOR_CONSTANT(CERROR, 0xFF0000FF);

#undef COLOR_CONSTANT
    } // namespace colors

    inline float scale(int val) { return val * style::scaling_factor; }
    inline float scale(float val) { return val * style::scaling_factor; }
    inline ImVec2 scale(const ImVec2& val) { return val * style::scaling_factor; }
    inline float operator""_sc(unsigned long long val) { return val * style::scaling_factor; }
    inline float operator""_sc(long double val) { return static_cast<float>(val) * style::scaling_factor; }

    constexpr ImVec2 V0 = ImVec2{0, 0};

    void pingFonts();

    class ScopeGuard {
        std::function<void()> f;

    public:
        explicit ScopeGuard(std::function<void()> f) : f(std::move(f)) {}
        ~ScopeGuard() { f(); }

        ScopeGuard(ScopeGuard&) = delete;
        ScopeGuard& operator=(const ScopeGuard&) = delete;

        class Ctor {
        public:
            ScopeGuard operator+(std::function<void()> fn) const { return ScopeGuard(std::move(fn)); }
        };
    };

#define SCOPEGUARDCAT2(A, B) A##B
#define SCOPEGUARDCAT(A, B) SCOPEGUARDCAT2(A, B)
#define SCOPE_EXIT [[maybe_unused]] ScopeGuard SCOPEGUARDCAT(GUARD, __COUNTER__) = ScopeGuard::Ctor() + [&]()
} // namespace LRI::RCI

#endif // LRI_CONTROL_PANEL_GUTILS_H
