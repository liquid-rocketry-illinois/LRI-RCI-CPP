#ifndef SETUP_H
#define SETUP_H

#include "GLFW/glfw3.h"
#include "imgui.h"

namespace LRI {
    static constexpr ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoDecoration;

    void imgui_init(GLFWwindow* window);
    void imgui_prerender();
    void imgui_postrender(GLFWwindow* window);
    void imgui_shutdown(GLFWwindow* window);
}

#endif //SETUP_H
