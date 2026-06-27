#include "UI/window.h"

#include <chrono>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include "stb_image.h"

#define GLFW_EXPOSE_NATIVE_WIN32
#include "UI/style.h"
#include "glfw/glfw3native.h"

// using namespace std::chrono_literals;

namespace LRI::RCI::splash {
    namespace {
        constexpr int SPLASH_WIDTH = 640;
        constexpr int SPLASH_HEIGHT = 400;
        constexpr std::string_view SPLASH_NAME = "Rocket Control Interface";

        // Only show splash for one second on debug build bc i aint got that kinda time to wait around
        constexpr std::chrono::milliseconds TIME_OPEN{
#ifdef RCIDEBUG
            1000
#else
            5000
#endif
        };

        std::chrono::system_clock::time_point startTime;
        GLFWwindow* window = nullptr;
    } // namespace

    void show() {
        // Setup creation hints
        glfwDefaultWindowHints();

        // no titlebar for splash
        glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);

        // Transparent framebuffer will mean the transparent regions in the bird png will show
        glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, GLFW_TRUE);

        // Disable moving and resizing window, and force on top
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
        glfwWindowHint(GLFW_FLOATING, GLFW_TRUE);
        glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);

        // Make the window
        window = glfwCreateWindow(SPLASH_WIDTH, SPLASH_HEIGHT, &SPLASH_NAME[0], nullptr, nullptr);
        glfwMakeContextCurrent(window);
        glfwSwapInterval(1);

        // Setup imgui
        ImGui::CreateContext();
        ImGui::GetIO().IniFilename = nullptr;
        ImGui::StyleColorsDark();
        ImGui_ImplGlfw_InitForOpenGL(window, true);
        ImGui_ImplOpenGL3_Init();

        // Setup the icon and load bird texture into gpu
        style::setWindowIcon(window);
        style::setupBirdIcon();

        // Disable the window icon showing in the taskbar
        HWND handle = glfwGetWin32Window(window);
        SetWindowLong(handle, GWL_EXSTYLE, WS_EX_PALETTEWINDOW);

        // Determine screen coordinates to place the window at so it is centered on the main monitor
        GLFWmonitor* mon = glfwGetPrimaryMonitor();

        {
            float xs, ys;
            glfwGetMonitorContentScale(mon, &xs, &ys);
            style::setScaleFactor((xs + ys) / 2);
        }

        const GLFWvidmode* mode = glfwGetVideoMode(mon);
        int mX = 0, mY = 0;
        glfwGetMonitorPos(mon, &mX, &mY);

        int winSize = static_cast<int>(500_sc);
        glfwSetWindowSize(window, winSize, winSize);

        int wX = mX + (mode->width - winSize) / 2;
        int wY = mY + (mode->height - winSize) / 2;

        glfwSetWindowPos(window, wX, wY);

        // Show and render window
        glfwShowWindow(window);

        startTime = std::chrono::system_clock::now();

        while(!glfwWindowShouldClose(window)) {
            glfwPollEvents();
            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();

            glfwGetWindowSize(window, &wX, &wY);

            // Show the image without an imgui window
            ImGui::GetForegroundDrawList()->AddImage(style::getBirdTex(), V0,
                                                     {static_cast<float>(wX), static_cast<float>(wY)});

            ImGui::Render();
            glViewport(0, 0, wX, wY);
            glClearColor(0, 0, 0, 0);
            glClear(GL_COLOR_BUFFER_BIT);

            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
            glfwSwapBuffers(window);

            // Close the splash after TIME_OPEN. surprise! splash isnt actually functional its just style points
            if(std::chrono::system_clock::now() - startTime > TIME_OPEN) glfwSetWindowShouldClose(window, GLFW_TRUE);
        }

        // Cleanup for the main window
        style::cleanupBirdTex();
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
        glfwDestroyWindow(window);
    }
} // namespace LRI::RCI::splash
