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

    ImVec2 scale(ImVec2 vec);
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
}

#endif //UTILS_H
