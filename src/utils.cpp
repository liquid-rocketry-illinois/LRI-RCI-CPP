#include "utils.h"

#include <implot.h>

#include "imgui.h"
#include "imgui_impl_opengl3.h"
#include "imgui_impl_glfw.h"

#include "WindowsResource.h"

#include "UI/TargetChooser.h"

// A mish-mash of various different things that are useful
namespace LRI::RCI {
    // Fonts
    ImFont* font_regular;
    ImFont* font_bold;

    // Scaling factor for hidpi screens
    float scaling_factor;

    // Function that initializes the rest of glfw, imgui, implot, and the fonts
    void imgui_init(GLFWwindow* window) {
        // Set window as current context, enable vsync, give it a title.
        // TODO: window icon
        glfwMakeContextCurrent(window);
        glfwSwapInterval(1);
        glfwSetWindowTitle(window, "LRI Rocket Control Panel");

        // Create imgui and implot contexts
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImPlot::CreateContext();
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

        // Load the fonts and add them to imgui. Ubuntu mono my beloved
        WindowsResource fonts("font-regular.ttf", "TTFFONT");
        font_regular = io.Fonts->AddFontFromMemoryTTF((void*) fonts.getData(), static_cast<int>(fonts.getSize()),
                                                      16 * scaling_factor, &fontConfig);
        fonts = WindowsResource("font-bold.ttf", "TTFFONT");
        font_bold = io.Fonts->AddFontFromMemoryTTF((void*) fonts.getData(), static_cast<int>(fonts.getSize()),
                                                   16 * scaling_factor, &fontConfig);

        // Start the TargetChooser window
        TargetChooser::getInstance();
    }

    // Is called to set up each frame before rendering
    void imgui_prerender(GLFWwindow* window) {
        glfwPollEvents();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
    }

    // Is called after each frame to draw the framebuffer and swap it to the screen
    void imgui_postrender(GLFWwindow* window) {
        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(BACKGROUND_COLOR.x, BACKGROUND_COLOR.y, BACKGROUND_COLOR.z, BACKGROUND_COLOR.w);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }

    // All shutdown functions for imgui, implot, and glfw
    void imgui_shutdown(GLFWwindow* window) {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImPlot::DestroyContext();
        ImGui::DestroyContext();

        glfwDestroyWindow(window);
        glfwTerminate();
    }

    ImVec2 scale(const ImVec2& vec) {
        return vec * scaling_factor;
    }

    float scale(float val) {
        return val * scaling_factor;
    }

    // Stopwatch class implementation
    StopWatch::StopWatch() {
        time(&lastClock);
    }

    void StopWatch::reset() {
        time(&lastClock);
    }

    time_t StopWatch::timeSince() const {
        time_t now;
        time(&now);
        return now - lastClock;
    }

    ImVec2 operator+(ImVec2 const& v1, ImVec2 const& v2) {
        return {v1.x + v2.x, v1.y + v2.y};
    }

    ImVec2 operator-(ImVec2 const& v1, ImVec2 const& v2) {
        return {v1.x - v2.x, v1.y - v2.y};
    }

    ImVec2 operator*(ImVec2 const& v1, ImVec2 const& v2) {
        return {v1.x * v2.x, v1.y * v2.y};
    }

    ImVec2 operator*(ImVec2 const& v1, const float constant) {
        return {v1.x * constant, v1.y * constant};
    }

    ImVec2 operator/(ImVec2 const& v1, ImVec2 const& v2) {
        return {v1.x / v2.x, v1.y / v2.y};
    }

    ImVec2 operator/(ImVec2 const& v1, const float constant) {
        return {v1.x / constant, v1.y / constant};
    }

}
