#include "GLFW/glfw3.h"
#include "imgui.h"
#include "setup.h"



int main() {
    if(!glfwInit()) {
        return -1;
    }

    glfwWindowHint(GLFW_MAXIMIZED, GLFW_TRUE);
    GLFWwindow* window = glfwCreateWindow(1920, 1080, "LRI RCP", nullptr, nullptr);
    if(!window) {
        return -1;
    }

    LRI::imgui_init(window);

    while(!glfwWindowShouldClose(window)) {
        LRI::imgui_prerender();

        ImGui::SetNextWindowPos(ImVec2(0, 0));
        ImGui::SetNextWindowSize(ImGui::GetMainViewport()->Size);
        if(ImGui::Begin("LRI RCP", nullptr, LRI::window_flags)) {
            ImGui::Text("LRI Rocket Control Panel");
        }

        ImGui::End();
        LRI::imgui_postrender(window);
    }

    LRI::imgui_shutdown(window);

    return 0;
}
