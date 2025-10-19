#ifndef LRI_CONTROL_PANEL_RENDERING_H
#define LRI_CONTROL_PANEL_RENDERING_H

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#endif

#include "GLFW/glfw3.h"

namespace LRI::RCI {
    // Helpers to organize the various ImGui related calls
    void imgui_init(GLFWwindow* window);
    void imgui_prerender();
    void imgui_postrender(GLFWwindow* window);
    void imgui_shutdown(GLFWwindow* window);
} // namespace LRI::RCI

#endif // LRI_CONTROL_PANEL_RENDERING_H
