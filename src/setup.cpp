#include "setup.h"
#include "imgui.h"
#include "imgui_impl_opengl3.h"
#include "imgui_impl_glfw.h"

namespace LRI {
    ImFont* font_regular;
    ImFont* font_bold;
    float scaling_factor;

    void imgui_init(GLFWwindow* window) {
        glfwMakeContextCurrent(window);
        glfwSwapInterval(0);
        glfwSetWindowTitle(window, "LRI Rocket Control Panel");

        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad; // Enable Gamepad Controls
        io.IniFilename = nullptr;

        ImGui::StyleColorsDark();

        ImGui_ImplGlfw_InitForOpenGL(window, true);
        ImGui_ImplOpenGL3_Init();

        glfwGetWindowContentScale(window, &scaling_factor, nullptr);
        io.Fonts->Clear();
        ImFontConfig fontConfig;
        fontConfig.FontDataOwnedByAtlas = false;
        font_regular = io.Fonts->AddFontFromFileTTF("font-regular.ttf", 16 * scaling_factor, &fontConfig);
        font_bold = io.Fonts->AddFontFromFileTTF("font-bold.ttf", 16 * scaling_factor, &fontConfig);

    }

    void imgui_prerender(GLFWwindow* window) {
        glfwPollEvents();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
    }

    void imgui_postrender(GLFWwindow* window) {
        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }

    void imgui_shutdown(GLFWwindow* window) {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();

        glfwDestroyWindow(window);
        glfwTerminate();
    }
}
