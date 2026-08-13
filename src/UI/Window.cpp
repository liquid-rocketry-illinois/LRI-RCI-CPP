#include "UI/Window.h"

#include <dwmapi.h>

#define GLFW_EXPOSE_NATIVE_WIN32
#include "glfw/glfw3native.h"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "implot.h"

#include "UI/Windowlet.h"
#include "UI/gutils.h"

namespace LRI::RCI {
    Window::Window() : window(nullptr), classid(0) {
        glfwDefaultWindowHints();
        glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
        glfwWindowHint(GLFW_MAXIMIZED, GLFW_TRUE);

        window = glfwCreateWindow(1280, 720, "LRI Rocket Control Interface", nullptr, style::getSharedResources());
        glfwMakeContextCurrent(window);
        glfwSwapInterval(1);

        // Create imgui and implot contexts
        IMGUI_CHECKVERSION();
        ImGui::SetCurrentContext(nullptr);
        ImGui::CreateContext(style::getSharedFonts());
        ImPlot::CreateContext();

        ImGuiIO& io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad; // Enable Gamepad Controls
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable | ImGuiConfigFlags_ViewportsEnable;
        io.ConfigDpiScaleFonts = true;
        io.ConfigDpiScaleViewports = true;
        io.IniFilename = nullptr;

        HWND hwnd = glfwGetWin32Window(window);
        // Turn window caption dark
        {
            BOOL t = true;
            DwmSetWindowAttribute(hwnd, DWMWINDOWATTRIBUTE::DWMWA_USE_IMMERSIVE_DARK_MODE, &t, sizeof(t));
        }

        ImGui::StyleColorsDark();

        ImGui_ImplGlfw_InitForOpenGL(window, true);
        ImGui_ImplOpenGL3_Init();

        glfwGetWindowContentScale(window, &style::scaling_factor, nullptr);
        style::setWindowIcon(window);

        // // Start the TargetChooser window
        // ControlWindowlet::getInstance();
    }

    Window::~Window() {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImPlot::DestroyContext();
        ImGui::DestroyContext();
        glfwDestroyWindow(window);
    }

    void Window::show() {
        glfwShowWindow(window);

        // Prevent the user getting flashbanged before the first frame renders
        {
            int wX, wY;
            glfwGetWindowSize(window, &wX, &wY);
            glViewport(0, 0, wX, wY);
            glClearColor(0, 0, 0, 0);
            glClear(GL_COLOR_BUFFER_BIT);
            glfwSwapBuffers(window);
        }

        while(!glfwWindowShouldClose(window)) {
            for(const auto& f : preframes) f();
            preframes.clear();

            glfwPollEvents();
            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();
            pingFonts();

            for(const auto& w : windowlets) w->render();

            int display_w, display_h;
            glfwGetFramebufferSize(window, &display_w, &display_h);

            // Draw version string on bottom left of window
            ImGui::GetBackgroundDrawList()->AddText(
                ImGui::GetMainViewport()->Pos +
                    ImVec2(10_sc, ImGui::GetMainViewport()->Size.y - scale(style::verStringSize().y) - 10_sc),
                0x33FFFFFF, style::versionString().c_str());

            ImGui::Render();
            glViewport(0, 0, display_w, display_h);
            glClearColor(0, 0, 0, 0);
            glClear(GL_COLOR_BUFFER_BIT);

            float largestSquare = static_cast<float>(std::min(display_w, display_h));
            float coordX = largestSquare / display_w / 1.0f;
            float coordY = largestSquare / display_h / 1.0f;

            glEnable(GL_TEXTURE_2D);
            glBindTexture(GL_TEXTURE_2D, style::birdIcon());

            glBegin(GL_QUADS);
            glTexCoord2f(0, 0);
            glVertex2f(-coordX, coordY);
            glTexCoord2f(1, 0);
            glVertex2f(coordX, coordY);
            glTexCoord2f(1, 1);
            glVertex2f(coordX, -coordY);
            glTexCoord2f(0, 1);
            glVertex2f(-coordX, -coordY);
            glEnd();

            glDisable(GL_TEXTURE_2D);

            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

            GLFWwindow* backup_current_context = glfwGetCurrentContext();
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
            glfwMakeContextCurrent(backup_current_context);

            glfwSwapBuffers(window);
        }
    }

    void Window::preframe(std::function<void()> func) { preframes.emplace_back(std::move(func)); }

    void Window::registerWindowlet(Windowlet* w) {
        preframe([&] { windowlets.emplace(w); });
    }
    void Window::unregisterWindowlet(Windowlet* w) {
        preframe([&] { windowlets.erase(w); });
    }

    int Window::getClassid() { return classid++; }
} // namespace LRI::RCI
