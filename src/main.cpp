#include <Windows.h>
#include "GLFW/glfw3.h"
#include "imgui.h"
#include "utils.h"

#include "UI/BaseUI.h"

int main() {
    if(!glfwInit()) {
        return -1;
    }

    glfwWindowHint(GLFW_MAXIMIZED, GLFW_TRUE);
    GLFWwindow* window = glfwCreateWindow(1920, 1080, "LRI RCI", nullptr, nullptr);
    if(!window) {
        return -1;
    }

    LRI::RCI::imgui_init(window);

    while(!glfwWindowShouldClose(window)) {
        LRI::RCI::imgui_prerender(window);
        LRI::RCI::renderWindows();
        LRI::RCI::imgui_postrender(window);
    }

    LRI::RCI::imgui_shutdown(window);

    return 0;
}