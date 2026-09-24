#ifndef LRI_CONTROL_PANEL_GUTILS_H
#define LRI_CONTROL_PANEL_GUTILS_H

#include <Windows.h>
#include <functional>
#include <string>

#include "glfw/glfw3.h"
#include "imgui.h"
#include "implot.h"

#include "hardware/EventLog.h"
#include "hardware/HardwareQualifier.h"

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

    std::string renderLatestReadingsString(const HardwareChannel& qual, float data);
    ImPlotPoint implotTargetFloat(int index, void* data);

    namespace GraphInfo {
        /*
         * These two structures are used for encoding the naming and rendering information for
         * a particular device class. This data is stored in the GRAPHINFO map. For each
         * device class, we maintain two lists:
         *  - The first contains the axis data. This is a list of the different axes/units belonging to the particular
         *    device class. This list also holds names for the seperate graphs that must be created in classic mode that
         *    correspond to each unique unit of measurement.
         *  - The second list contains data about each channel. It first identifies what unit/axis this channel may be
         *    graphed on. This is an integer representing an index into the first list. The second value is part of the
         *    string that may be used for the name of the line itself within the graph. In classic mode, this value is
         *    used as-is for the name that will appear in the legend for this line. In multi mode, the device name is
         *    prepended so each line can be matched to the device it belongs to.
         *
         * All this data is stored in the GRAPHINFO map, whose definition is present in the gutils.cpp
         * file. It is truly a horrendous sight to behold, but its the best way I could think of to encode all
         * this data in the program. It does allow for some very clean loops that actually render the graphs,
         * though, as opposed to the previous system (in v1.0.x) which has a bunch of special cases for each
         * device class, and the v1.3.x system which had *two* of these awful structures storing the data in slightly
         * different ways for each the classic and multi modes.
         */
        struct Line {
            size_t axis;
            std::string legend;
        };

        struct Axis {
            std::string name;
            std::string independentGraphName;
        };

        struct GraphInfo {
            std::vector<Axis> axes;
            std::vector<Line> lines;
        };

        extern const std::map<const RCP_DeviceClass, const GraphInfo> GRAPHINFO;
    } // namespace GraphInfo
} // namespace LRI::RCI

#endif // LRI_CONTROL_PANEL_GUTILS_H
