#include <Windows.h>

#include "utils.h"
#include "imgui.h"
#include "imgui_impl_opengl3.h"
#include "imgui_impl_glfw.h"

#include "WindowsResource.h"

#include "UI/TargetChooser.h"
#include "UI/EStop.h"

namespace LRI::RCI {
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

        WindowsResource fonts("font-regular.ttf", "TTFFONT");
        font_regular = io.Fonts->AddFontFromMemoryTTF((void*) fonts.getData(), fonts.getSize(), 16 * scaling_factor, &fontConfig);
        fonts = WindowsResource("font-bold.ttf", "TTFFONT");
        font_bold = io.Fonts->AddFontFromMemoryTTF((void*) fonts.getData(), fonts.getSize(), 16 * scaling_factor, &fontConfig);

        TargetChooser::getInstance();
        EStop::getInstance();
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
        glClearColor(0.3f, 0.0f, 0.6f, 1.0f);
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

    ImVec2 scale(ImVec2 vec) {
        return ImVec2(vec.x * scaling_factor, vec.y * scaling_factor);
    }

    float scale(float val) {
        return val * scaling_factor;
    }

    StopWatch::StopWatch() {
        time(&lastClock);
    }

    void StopWatch::reset() {
        time(&lastClock);
    }

    time_t StopWatch::timeSince() {
        time_t now;
        time(&now);
        return now - lastClock;
    }



}
