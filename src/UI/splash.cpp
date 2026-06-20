#include "UI/splash.h"

#include <print>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include "stb_image.h"

#define GLFW_EXPOSE_NATIVE_WIN32
#include "UI/style.h"
#include "glfw/glfw3native.h"

namespace LRI::RCI {
    namespace {
        constexpr int SPLASH_WIDTH = 640;
        constexpr int SPLASH_HEIGHT = 400;
        constexpr std::string_view SPLASH_NAME = "Rocket Control Interface";
    } // namespace

    SplashWindow::SplashWindow() : window(nullptr), startTime(std::chrono::system_clock::now()) {
        glfwDefaultWindowHints();

        glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);
        glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, GLFW_TRUE);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
        glfwWindowHint(GLFW_FLOATING, GLFW_TRUE);
        glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);

        window = glfwCreateWindow(SPLASH_WIDTH, SPLASH_HEIGHT, &SPLASH_NAME[0], nullptr, nullptr);

        glfwMakeContextCurrent(window);
        glfwSwapInterval(1);

        ImGui::CreateContext();
        ImGui::GetIO().IniFilename = nullptr;
        ImGui::StyleColorsDark();
        ImGui_ImplGlfw_InitForOpenGL(window, true);
        ImGui_ImplOpenGL3_Init();

        style::setWindowIcon(window);
        style::setupBirdIcon();

        HWND handle = glfwGetWin32Window(window);
        SetWindowLong(handle, GWL_EXSTYLE, WS_EX_PALETTEWINDOW);

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

        glfwShowWindow(window);
    }

    void SplashWindow::loop() {
        while(!glfwWindowShouldClose(window)) {
            glfwPollEvents();
            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();

            int wX, wY;
            glfwGetWindowSize(window, &wX, &wY);

            ImGui::GetForegroundDrawList()->AddImage(style::getBirdTex(), {0, 0}, {static_cast<float>(wX), static_cast<float>(wY)});

            ImGui::Render();
            glViewport(0, 0, wX, wY);
            glClearColor(0, 0, 0, 0);
            glClear(GL_COLOR_BUFFER_BIT);

            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
            glfwSwapBuffers(window);

            if(std::chrono::system_clock::now() - startTime > TIME_OPEN) glfwSetWindowShouldClose(window, GLFW_TRUE);
        }
    }

    SplashWindow::~SplashWindow() {
        style::cleanupBirdTex();
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
        glfwDestroyWindow(window);
    }
} // namespace LRI::RCI
