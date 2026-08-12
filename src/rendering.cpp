#include "rendering.h"

#include <dwmapi.h>
#include <string>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "implot.h"

#include "EmbeddedResource.h"
#include "VERSION.h"

#include "UI/Windowlet.h"
#include "UI/gutils.h"

#define GLFW_EXPOSE_NATIVE_WIN32
#include "GLFW/glfw3native.h"

namespace LRI::RCI {
    static std::string VERSION_STRING;

    // Function that initializes the rest of glfw, imgui, implot, and the fonts
    void imgui_init(GLFWwindow* window) {
        VERSION_STRING = "RCI ";
        VERSION_STRING += std::string(RCI_VERSION, RCI_VERSION_END) +=
            "\nRCP " + std::string(RCP_VERSION, RCP_VERSION_END);

        // Set window as current context, enable vsync, give it a title.
        glfwMakeContextCurrent(window);
        glfwSwapInterval(1);
        glfwSetWindowTitle(window, "LRI Rocket Control Panel");

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


        // Turn window caption dark
        {
            BOOL t = true;
            DwmSetWindowAttribute(glfwGetWin32Window(window), DWMWINDOWATTRIBUTE::DWMWA_USE_IMMERSIVE_DARK_MODE, &t,
                                  sizeof(t));
        }

        ImGui::StyleColorsDark();

        ImGui_ImplGlfw_InitForOpenGL(window, true);
        ImGui_ImplOpenGL3_Init();

        glfwGetWindowContentScale(window, &style::scaling_factor, nullptr);
        style::setWindowIcon(window);

        // Start the TargetChooser window
        ControlWindowlet::getInstance();
    }

    static std::string iniFilePath;
    static bool setIniFile = false;

    void setIniFileForNextFrame(const std::string& path) {
        setIniFile = true;
        iniFilePath = path;
    }

    // Is called to set up each frame before rendering
    void imgui_prerender() {
        // If a configuration has been loaded and requests to load a window layout, do that here
        // before the new frame.
        if(setIniFile) {
            ImGui::LoadIniSettingsFromDisk(iniFilePath.c_str());
            setIniFile = false;
        }

        glfwPollEvents();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        pingFonts();
    }

    // Is called after each frame to draw the framebuffer and swap it to the screen
    void imgui_postrender(GLFWwindow* window) {
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);

        // Draw version string on bottom left of window
        ImGui::GetBackgroundDrawList()->AddText(
            ImGui::GetMainViewport()->Pos + ImVec2(5_sc, ImGui::GetMainViewport()->Size.y - 40_sc), 0x33FFFFFF,
            VERSION_STRING.c_str(), VERSION_STRING.c_str() + VERSION_STRING.length());

        ImGui::Render();
        glViewport(0, 0, display_w, display_h);
        glClearColor(BACKGROUND_COLOR.x, BACKGROUND_COLOR.y, BACKGROUND_COLOR.z, BACKGROUND_COLOR.w);
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

    // All shutdown functions for imgui, implot, and glfw
    void imgui_shutdown(GLFWwindow* window) {
        ControlWindowlet::getInstance()->cleanup();
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImPlot::DestroyContext();
        ImGui::DestroyContext();

        glfwDestroyWindow(window);
    }

    void render(GLFWwindow* window) {
        imgui_prerender();
        Windowlet::renderWindowlets();
        imgui_postrender(window);
    }
} // namespace LRI::RCI
