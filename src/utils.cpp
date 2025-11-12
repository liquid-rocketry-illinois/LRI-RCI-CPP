#include "utils.h"

#include <shlobj_core.h>

#include <implot.h>
#include "RCP_Host/RCP_Host.h"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include "UI/Windowlet.h"
#include "VERSION.h"
#include "WindowsResource.h"

// A mish-mash of various different things that are useful
namespace LRI::RCI {
    // Fonts
    ImFont* font_regular;
    ImFont* font_bold;
    ImFont* font_italic;

    // Scaling factor for hidpi screens
    float scaling_factor;
    IniFilePath iniFilePath;

    static std::string VERSION_STRING;

    // Implementation for IniFilePath
    bool IniFilePath::empty() const { return path.empty(); }

    std::string IniFilePath::getPath() {
        std::string copy = path;
        path = "";
        return copy;
    }

    // Function that initializes the rest of glfw, imgui, implot, and the fonts
    void imgui_init(GLFWwindow* window) {
        VERSION_STRING = "RCI ";
        VERSION_STRING += std::string(RCI_VERSION, RCI_VERSION_END) +=
            "\nRCP " + std::string(RCP_VERSION, RCP_VERSION_END);

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
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
        // io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
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

        fonts = WindowsResource("font-italic.ttf", "TTFFONT");
        font_italic = io.Fonts->AddFontFromMemoryTTF((void*) fonts.getData(), static_cast<int>(fonts.getSize()),
                                                     16 * scaling_factor, &fontConfig);

        // Start the TargetChooser window
        ControlWindowlet::getInstance();
    }

    // Is called to set up each frame before rendering
    void imgui_prerender(GLFWwindow* window) {
        // If a configuration has been loaded and requests to load a window layout, do that here
        // before the new frame.
        if(!iniFilePath.empty()) {
            ImGui::LoadIniSettingsFromDisk(iniFilePath.getPath().c_str());
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
        glfwTerminate();
    }

    ImVec2 scale(const ImVec2& vec) { return vec * scaling_factor; }

    float scale(float val) { return val * scaling_factor; }

    // Stopwatch class implementation
    StopWatch::StopWatch() : lastClock(std::chrono::system_clock::now()) {}

    void StopWatch::reset() { lastClock = std::chrono::system_clock::now(); }

    float StopWatch::timeSince() const {
        const std::chrono::duration<float> elapsed = std::chrono::system_clock::now() - lastClock;
        return elapsed.count();
    }

    std::string devclassToString(RCP_DeviceClass devclass) {
        switch(devclass) {
        case RCP_DEVCLASS_TEST_STATE:
            return "Test State (Virtual Device)";

        case RCP_DEVCLASS_SIMPLE_ACTUATOR:
            return "Simple Actuator";

        case RCP_DEVCLASS_STEPPER:
            return "Stepper Motor";

        case RCP_DEVCLASS_CUSTOM:
            return "Raw Data (Virtual Device)";

        case RCP_DEVCLASS_AM_PRESSURE:
            return "Ambient Pressure";

        case RCP_DEVCLASS_AM_TEMPERATURE:
            return "Ambient Temperature";

        case RCP_DEVCLASS_PRESSURE_TRANSDUCER:
            return "Pressure Transducer";

        case RCP_DEVCLASS_RELATIVE_HYGROMETER:
            return "Relative Hygrometer";

        case RCP_DEVCLASS_LOAD_CELL:
            return "Load Cell (weight)";

        case RCP_DEVCLASS_POWERMON:
            return "Power Monitor";

        case RCP_DEVCLASS_ACCELEROMETER:
            return "Accelerometer";

        case RCP_DEVCLASS_GYROSCOPE:
            return "Gyroscope";

        case RCP_DEVCLASS_MAGNETOMETER:
            return "Magnetometer";

        case RCP_DEVCLASS_GPS:
            return "GPS";

        default:
            return "Unknown";
        }
    }

    ImVec2 operator+(ImVec2 const& v1, ImVec2 const& v2) { return {v1.x + v2.x, v1.y + v2.y}; }

    ImVec2 operator-(ImVec2 const& v1, ImVec2 const& v2) { return {v1.x - v2.x, v1.y - v2.y}; }

    ImVec2 operator*(ImVec2 const& v1, ImVec2 const& v2) { return {v1.x * v2.x, v1.y * v2.y}; }

    ImVec2 operator*(ImVec2 const& v1, const float constant) { return {v1.x * constant, v1.y * constant}; }

    ImVec2 operator/(ImVec2 const& v1, ImVec2 const& v2) { return {v1.x / v2.x, v1.y / v2.y}; }

    ImVec2 operator/(ImVec2 const& v1, const float constant) { return {v1.x / constant, v1.y / constant}; }

    static std::filesystem::path roamingFolder;

    const std::filesystem::path& getRoamingFolder() {
        return roamingFolder;
    }

    void detectRoamingFolder() {
#ifdef RCIDEBUG
        char buf[256];
        DWORD retlen = GetModuleFileName(nullptr, buf, sizeof(buf));
        if(retlen >= sizeof(buf)) std::exit(-1);
        roamingFolder = buf;
        roamingFolder = roamingFolder.parent_path() / "roaming";
#else
        PWSTR pathstr = nullptr;
        HRESULT res = SHGetKnownFolderPath(FOLDERID_RoamingAppData, 0, nullptr, &pathstr);
        if(res != S_OK || !pathstr) std::exit(-1);
        roamingFolder = pathstr;
        roamingFolder /= "LRI Electronics";
        roamingFolder /= "Rocket Control Interface (RCI)";
        CoTaskMemFree(pathstr);
#endif

        if(std::filesystem::exists(roamingFolder)) {
            if(!std::filesystem::is_directory(roamingFolder)) {
                std::exit(-1);
            }
        }

        else std::filesystem::create_directories(roamingFolder);

        auto targetsFolder = roamingFolder / "targets";
        if(std::filesystem::exists(targetsFolder)) {
            if(!std::filesystem::is_directory(targetsFolder)) std::exit(-1);
        }

        else std::filesystem::copy("targets", roamingFolder / "targets");
    }

} // namespace LRI::RCI

namespace ImGui {
    // See utils.h
    bool TimedButton(const char* label, LRI::RCI::StopWatch& sw, const ImVec2& size) {
        Button(label, size);
        if(IsItemActivated()) sw.reset();
        return IsItemActive();
    }

    TimedButton::TimedButton(const char* label) : label(label), clicked(false) {}

    bool TimedButton::render() {
        Button(label);
        clicked = IsItemActive();
        if(IsItemActivated()) timer.reset();
        return clicked;
    }

    float TimedButton::getHoldTime() const { return clicked ? timer.timeSince() : 0; }
} // namespace ImGui
