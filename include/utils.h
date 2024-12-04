#ifndef UTILS_H
#define UTILS_H

#include <Windows.h>
#include "GLFW/glfw3.h"
#include "imgui.h"

namespace LRI::RCI {
    extern ImFont* font_regular;
    extern ImFont* font_bold;
    extern float scaling_factor;
    extern HANDLE com;

    ImVec2 scale(const ImVec2& vec);
    float scale(float val);
    void imgui_init(GLFWwindow* window);
    void imgui_prerender(GLFWwindow* window);
    void imgui_postrender(GLFWwindow* window);
    void imgui_shutdown(GLFWwindow* window);

    void renderCOMChooser();
    void renderEStop();

    class StopWatch final {
        time_t lastClock;

    public:
        StopWatch();
        ~StopWatch() = default;

        void reset();
        time_t timeSince();
    };

    // ImVec2 operators
    ImVec2 operator+(ImVec2 const& v1, ImVec2 const& v2);
    ImVec2 operator-(ImVec2 const& v1, ImVec2 const& v2);
    ImVec2 operator*(ImVec2 const& v1, ImVec2 const& v2);
    ImVec2 operator*(ImVec2 const& v1, float constant);
    ImVec2 operator/(ImVec2 const& v1, ImVec2 const& v2);
    ImVec2 operator/(ImVec2 const& v1, float constant);

}

#endif //UTILS_H
