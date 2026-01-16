#include "rendering.h"

#include <dwmapi.h>
#include <string>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "implot.h"

#include "EmbeddedResource.h"
#include "VERSION.h"
#include "utils.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define GLFW_EXPOSE_NATIVE_WIN32
#include "GLFW/glfw3native.h"

#include "fontawesome.h"

namespace LRI::RCI {
    static std::string VERSION_STRING;
    static GLuint iconTex;

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
        ImGui::CreateContext();
        ImPlot::CreateContext();
        ImGuiIO& io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad; // Enable Gamepad Controls
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
        // io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
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

        glfwGetWindowContentScale(window, &scaling_factor, nullptr);
        io.Fonts->Clear();

        // Some constants
        const float fontSize = scale(16);
        ImFontConfig fontConfig;
        fontConfig.FontDataOwnedByAtlas = false;

        // Load the fonts and add them to imgui. Ubuntu mono my beloved
        EmbeddedResource res("font_regular.ttf");
        font_regular = io.Fonts->AddFontFromMemoryTTF((void*) res.getData(), static_cast<int>(res.getLength()),
                                                      fontSize, &fontConfig);

        // Add fontawesome as merged into the regulars
        static constexpr ImWchar glyphs[] = {ICON_MIN_FA, ICON_MAX_16_FA, 0};
        ImFontConfig faconfig;
        faconfig.MergeMode = true;
        faconfig.PixelSnapH = true;
        faconfig.GlyphMinAdvanceX = fontSize;
        faconfig.FontDataOwnedByAtlas = false;
        res = EmbeddedResource("fa_900.ttf");
        io.Fonts->AddFontFromMemoryTTF((void*) res.getData(), static_cast<int>(res.getLength()), fontSize, &faconfig,
                                       glyphs);


        res = EmbeddedResource("font_bold.ttf");
        font_bold = io.Fonts->AddFontFromMemoryTTF((void*) res.getData(), static_cast<int>(res.getLength()), fontSize,
                                                   &fontConfig);

        res = EmbeddedResource("font_italic.ttf");
        font_italic = io.Fonts->AddFontFromMemoryTTF((void*) res.getData(), static_cast<int>(res.getLength()), fontSize,
                                                     &fontConfig);

        // Load logo for the window icon
        res = EmbeddedResource("LRI_Logo.png");
        GLFWimage image;
        image.pixels = stbi_load_from_memory((unsigned char*) res.getData(), static_cast<int>(res.getLength()),
                                             &image.width, &image.height, nullptr, 4);
        glfwSetWindowIcon(window, 1, &image);
        stbi_image_free(image.pixels);

        // Load logo for the background image
        res = EmbeddedResource("LRI_Logo_big.png");
        int imw, imh;
        unsigned char* imaged = stbi_load_from_memory((unsigned char*) res.getData(), static_cast<int>(res.getLength()),
                                                      &imw, &imh, nullptr, 4);

        // Set up background image texture
        glGenTextures(1, &iconTex);
        glBindTexture(GL_TEXTURE_2D, iconTex);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, imw, imh, 0, GL_RGBA, GL_UNSIGNED_BYTE, imaged);
        stbi_image_free(imaged);
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
    }

    // Is called after each frame to draw the framebuffer and swap it to the screen
    void imgui_postrender(GLFWwindow* window) {
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);

        // Draw version string on bottom left of window
        ImGui::GetBackgroundDrawList()->AddText(
            ImGui::GetMainViewport()->Pos + ImVec2(scale(5), ImGui::GetMainViewport()->Size.y - scale(40)), 0x33FFFFFF,
            VERSION_STRING.c_str(), VERSION_STRING.c_str() + VERSION_STRING.length());

        ImGui::Render();
        glViewport(0, 0, display_w, display_h);
        glClearColor(BACKGROUND_COLOR.x, BACKGROUND_COLOR.y, BACKGROUND_COLOR.z, BACKGROUND_COLOR.w);
        glClear(GL_COLOR_BUFFER_BIT);

        float largestSquare = static_cast<float>(std::min(display_w, display_h));
        float coordX = largestSquare / display_w / 1.0f;
        float coordY = largestSquare / display_h / 1.0f;

        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, iconTex);

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
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImPlot::DestroyContext();
        ImGui::DestroyContext();

        glfwDestroyWindow(window);
        glfwTerminate();
    }
} // namespace LRI::RCI
