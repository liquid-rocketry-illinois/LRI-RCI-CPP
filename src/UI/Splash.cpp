#include "UI/Splash.h"

#include <future>
#include <string>

#define GLFW_EXPOSE_NATIVE_WIN32
#include "glfw/glfw3native.h"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include "UI/gutils.h"

namespace {
    constexpr int SPLASH_SIZE = 500;
    constexpr std::string_view SPLASH_NAME = "RCI";
    constexpr std::chrono::milliseconds SPLASH_DURATION =
#ifdef RCIDEBUG
        std::chrono::milliseconds(1000);
#else
        std::chrono::milliseconds(5000);
#endif
} // namespace

namespace LRI::RCI {
    Splash::Splash() : window(nullptr) {
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
        window = glfwCreateWindow(SPLASH_SIZE, SPLASH_SIZE, &SPLASH_NAME[0], nullptr, style::getSharedResources());
        glfwMakeContextCurrent(window);
        glfwSwapInterval(1);

        // Setup imgui
        ImGui::CreateContext(style::getSharedFonts());
        ImGui::GetIO().IniFilename = nullptr;
        ImGui::StyleColorsDark();
        ImGui_ImplGlfw_InitForOpenGL(window, true);
        ImGui_ImplOpenGL3_Init();

        style::setWindowIcon(window);

        // Disable the window icon showing in the taskbar
        HWND handle = glfwGetWin32Window(window);
        SetWindowLong(handle, GWL_EXSTYLE, WS_EX_PALETTEWINDOW);

        // Determine screen coordinates to place the window at so it is centered on the main monitor
        GLFWmonitor* mon = glfwGetPrimaryMonitor();

        {
            float xs, ys;
            glfwGetMonitorContentScale(mon, &xs, &ys);
            style::scaling_factor = (xs + ys) / 2;
        }

        const GLFWvidmode* mode = glfwGetVideoMode(mon);
        int mX = 0, mY = 0;
        glfwGetMonitorPos(mon, &mX, &mY);

        int winSize = static_cast<int>(scale(SPLASH_SIZE));
        glfwSetWindowSize(window, winSize, winSize);

        int wX = mX + (mode->width - winSize) / 2;
        int wY = mY + (mode->height - winSize) / 2;

        glfwSetWindowPos(window, wX, wY);
    }

    void Splash::show() {
        glfwShowWindow(window);

        auto fut = std::async(std::launch::async, [] {
            detectRoamingFolder();
            enumSerialDevs();
        });
        timer.reset();

        while(!glfwWindowShouldClose(window)) {
            glfwPollEvents();
            pingFonts();
            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();

            int wX, wY;
            glfwGetWindowSize(window, &wX, &wY);

            // Show the image without an imgui window
            ImGui::GetForegroundDrawList()->AddImage(style::birdIcon(), V0,
                                                     {static_cast<float>(wX), static_cast<float>(wY)});

            ImGui::Render();
            glViewport(0, 0, wX, wY);
            glClearColor(0, 0, 0, 0);
            glClear(GL_COLOR_BUFFER_BIT);

            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
            glfwSwapBuffers(window);

            // Wait until both the setup tasks and the timer have finished
            if(timer.millis() > SPLASH_DURATION && fut.wait_for(std::chrono::seconds(0)) == std::future_status::ready)
                glfwSetWindowShouldClose(window, GLFW_TRUE);
        }

        fut.get();
    }

    Splash::~Splash() {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
        glfwDestroyWindow(window);
        style::resetFontFrameCount();
    }
} // namespace LRI::RCI
