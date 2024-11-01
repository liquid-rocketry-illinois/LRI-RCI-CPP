#ifndef SETUP_H
#define SETUP_H

#include "GLFW/glfw3.h"
#include "imgui.h"

namespace LRI::RCI {
    static constexpr ImGuiWindowFlags window_flags = ImGuiWindowFlags_None;

    extern ImFont* font_regular;
    extern ImFont* font_bold;
    extern float scaling_factor;

    void imgui_init(GLFWwindow* window);
    void imgui_prerender(GLFWwindow* window);
    void imgui_postrender(GLFWwindow* window);
    void imgui_shutdown(GLFWwindow* window);
}

#endif //SETUP_H
