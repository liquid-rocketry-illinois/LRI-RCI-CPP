#include "UI/Window.h"

#include <dwmapi.h>

#define GLFW_EXPOSE_NATIVE_WIN32
#include "glfw/glfw3native.h"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "implot.h"

#include "nlohmann/json.hpp"

#include "UI/Windowlet.h"
#include "UI/gutils.h"
#include "UI/vscode_icons.h"

#include "hardware/HardwareControl.h"
#include "hardware/json.h"

namespace LRI::RCI {
    Window::Window() : window(nullptr), oldProc(nullptr), chooser(this) {
        glfwDefaultWindowHints();
        glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
        glfwWindowHint(GLFW_MAXIMIZED, GLFW_TRUE);
        glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);

        window = glfwCreateWindow(1280, 720, "LRI Rocket Control Interface", nullptr, style::getSharedResources());

        HWND hwnd = glfwGetWin32Window(window);
        // Turn window caption dark
        {
            BOOL t = true;
            DwmSetWindowAttribute(hwnd, DWMWINDOWATTRIBUTE::DWMWA_USE_IMMERSIVE_DARK_MODE, &t, sizeof(t));
        }

        // Needed for rounded corners
        MARGINS m = {1, 1, 1, 1};
        DwmExtendFrameIntoClientArea(hwnd, &m);

        LONG style = GetWindowLong(hwnd, GWL_STYLE);
        style |= WS_OVERLAPPEDWINDOW;
        style &= ~WS_POPUP;
        SetWindowLong(hwnd, GWL_STYLE, style);

        SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));
        oldProc =
            reinterpret_cast<WNDPROC>(SetWindowLongPtr(hwnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(borderlessProc)));

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

        style::setColors();

        ImGui_ImplGlfw_InitForOpenGL(window, true);
        ImGui_ImplOpenGL3_Init();

        glfwGetWindowContentScale(window, &style::scaling_factor, nullptr);
        glfwSetWindowSizeLimits(window, static_cast<int>(900_sc), static_cast<int>(300_sc), GLFW_DONT_CARE,
                                GLFW_DONT_CARE);
        style::setWindowIcon(window);

        // // Start the TargetChooser window
        registerWindowlet(&chooser);
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

            renderBackground();
            renderTitlebar();

            for(const auto& w : windowlets) w->render();

            int display_w, display_h;
            glfwGetFramebufferSize(window, &display_w, &display_h);
            ImGui::Render();
            glViewport(0, 0, display_w, display_h);
            glClearColor(0, 0, 0, 1);
            glClear(GL_COLOR_BUFFER_BIT);

            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

            GLFWwindow* backup_current_context = glfwGetCurrentContext();
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
            glfwMakeContextCurrent(backup_current_context);

            glfwSwapBuffers(window);
        }
    }

    void Window::renderBackground() {
        const ImVec2 wPos = ImGui::GetMainViewport()->Pos;
        const ImVec2 wSize = ImGui::GetMainViewport()->Size;
        const ImVec2 blankSpace = wSize - ImVec2{0, scale(CAPTION_SIZE)};

        // Draw version string on bottom left of window
        ImGui::GetBackgroundDrawList()->AddText(ImGui::GetMainViewport()->Pos +
                                                    ImVec2(10_sc, wSize.y - scale(style::verStringSize().y) - 10_sc),
                                                0x33FFFFFF, style::versionString().c_str());

        ImVec2 impos;
        ImVec2 imsize;

        if(blankSpace.y < blankSpace.x) {
            float leftoverX = blankSpace.x - blankSpace.y;
            impos = {leftoverX / 2, scale(CAPTION_SIZE)};
            imsize = {blankSpace.y, blankSpace.y};
        }

        else {
            float leftoverY = blankSpace.y - blankSpace.x;
            impos = {0, scale(CAPTION_SIZE) + (leftoverY / 2)};
            imsize = {blankSpace.x, blankSpace.x};
        }

        ImGui::GetBackgroundDrawList()->AddImage(style::birdIcon(), wPos + impos, wPos + impos + imsize);
    }

    void Window::renderTitlebar() {
        constexpr ImGuiWindowFlags CAPTION_FLAGS =
            ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoDocking;
        const ImVec2 vPos = ImGui::GetMainViewport()->Pos;
        const ImVec2 vSize = ImGui::GetMainViewport()->Size;
        const float capheight = scale(CAPTION_SIZE);
        const ImVec2 capsize = ImVec2{vSize.x, capheight};
        const ImVec2 maxSquare = ImVec2{capheight, capheight};
        const float textY = (capheight - scale(16)) / 2;

        ImGui::SetNextWindowPos(vPos);
        ImGui::SetNextWindowSize(capsize);
        ImGui::SetNextWindowViewport(ImGui::GetMainViewport()->ID);

        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0);

        SCOPE_EXIT {
            ImGui::End();
            ImGui::PopStyleVar();
        };

        if(!ImGui::Begin("##windowcaption", nullptr, CAPTION_FLAGS)) return;

        ImGui::SetCursorPos(V0);
        ImGui::Image(style::birdIcon(), maxSquare);

        ImGui::SameLine(0, scale(10));
        ImGui::SetCursorPosY(textY);

        if(HWCTRL::targetOpen()) {
            ImGui::Text("%s | %s | Packet Buffer Size: %d | Polling Rate: ", openTarget.c_str(), openInterf.c_str(), 0);
            ImGui::SameLine(0, 5);
            ImGui::SetNextItemWidth(40_sc);
            ImGui::SetCursorPosY(textY);
            ImGui::InputInt("##pollrate", &HWCTRL::POLLS_PER_UPDATE, 0);
            if(HWCTRL::POLLS_PER_UPDATE < 1) HWCTRL::POLLS_PER_UPDATE = 1;

            ImGui::SameLine();
            ImGui::SetCursorPosY(textY);

            ImGui::Text(" | ");

            ImGui::SameLine();
            ImGui::SetCursorPosY(textY);

            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, colors::BUTTON_CLOSE_HOVERED);
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, colors::BUTTON_CLOSE_ACTIVE);

            if(ImGui::Button("CLOSE")) {
                preframe([&] {
                    registerWindowlet(&chooser);
                    HWCTRL::end();
                });
            }

            ImGui::PopStyleColor(2);
        }

        else {
            ImGui::Text("No Open Target " ICON_VS_ROCKET);
        }

        ImGui::PushStyleColor(ImGuiCol_Button, colors::CTRANSPARENT);
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, colors::LOW_SEMITRANSPARENT);
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, colors::HIGH_SEMITRANSPARENT);

        // We have 3 buttons to render aligned to right edge of screen, no spacing between them
        ImGui::SetCursorPos({capsize.x - 3 * maxSquare.x, 0});
        if(ImGui::Button(ICON_VS_CHROME_MINIMIZE "##minimize", maxSquare)) glfwIconifyWindow(window);

        // cursor.x += maxSquare.x;
        ImGui::SameLine(0, 0);
        if(glfwGetWindowAttrib(window, GLFW_MAXIMIZED)) {
            if(ImGui::Button(ICON_VS_CHROME_RESTORE "###restore", maxSquare)) glfwRestoreWindow(window);
        }

        else {
            if(ImGui::Button(ICON_VS_CHROME_MAXIMIZE "###restore", maxSquare)) glfwMaximizeWindow(window);
        }

        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, colors::BUTTON_CLOSE_HOVERED);
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, colors::BUTTON_CLOSE_ACTIVE);

        ImGui::SameLine(0, 0);
        if(ImGui::Button(ICON_VS_CHROME_CLOSE "##close", maxSquare)) glfwSetWindowShouldClose(window, GLFW_TRUE);

        ImGui::PopStyleColor(5);
    }

    void Window::preframe(std::function<void()> func) { preframes.emplace_back(std::move(func)); }

    void Window::registerWindowlet(Windowlet* w) { windowlets.emplace(w); }

    void Window::unregisterWindowlets() {
        preframe([&] {
            for(auto& w : windowlets) delete w;
            windowlets.clear();
        });
    }

    void Window::startTarget(RCP_Interface* interf, const TargetConfig& configPath) {
        openInterf = interf->interfaceType();

    }

    LRESULT Window::borderlessProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
        Window* w = reinterpret_cast<Window*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));

        switch(uMsg) {
        case WM_NCACTIVATE:
            return DefWindowProc(hwnd, uMsg, wParam, -1);

        case WM_NCPAINT:
            return DefWindowProc(hwnd, uMsg, wParam, lParam);

        case WM_NCCALCSIZE: {
            RECT& rect = *reinterpret_cast<RECT*>(lParam);
            RECT client = rect;

            CallWindowProc(w->oldProc, hwnd, uMsg, wParam, lParam);

            if(IsZoomed(hwnd)) {
                WINDOWINFO windowInfo = {};
                windowInfo.cbSize = sizeof(WINDOWINFO);
                GetWindowInfo(hwnd, &windowInfo);
                rect = RECT{.left = static_cast<LONG>(client.left + windowInfo.cyWindowBorders),
                            .top = static_cast<LONG>(client.top + windowInfo.cyWindowBorders),
                            .right = static_cast<LONG>(client.right - windowInfo.cyWindowBorders),
                            .bottom = static_cast<LONG>(client.bottom - windowInfo.cyWindowBorders) + 1};
            }

            else rect = client;
            return WVR_REDRAW;
        }

        case WM_ERASEBKGND:
            return 1;
        case WM_WINDOWPOSCHANGING: {
            // Make sure that windows discards the entire client area when resizing to avoid flickering
            const auto windowPos = reinterpret_cast<LPWINDOWPOS>(lParam);
            windowPos->flags |= SWP_NOCOPYBITS;
            break;
        }

        case WM_NCHITTEST: {
            if(ImGui::IsAnyItemHovered()) return HTCLIENT;
            POINT cursor = {((int) (short) LOWORD(lParam)), ((int) (short) HIWORD(lParam))};

            // Formatter goes berserk with this :(
            const POINT border{
                static_cast<LONG>(scale(GetSystemMetrics(SM_CXFRAME) + GetSystemMetrics(SM_CXPADDEDBORDER))),
                static_cast<LONG>(scale(GetSystemMetrics(SM_CYFRAME) + GetSystemMetrics(SM_CXPADDEDBORDER)))};


            RECT wpos;
            GetWindowRect(hwnd, &wpos);

            constexpr static auto RegionClient = 0b0000;
            constexpr static auto RegionLeft = 0b0001;
            constexpr static auto RegionRight = 0b0010;
            constexpr static auto RegionTop = 0b0100;
            constexpr static auto RegionBottom = 0b1000;

            const auto result = RegionLeft * (cursor.x < (wpos.left + border.x)) |
                RegionRight * (cursor.x >= (wpos.right - border.x)) | RegionTop * (cursor.y < (wpos.top + border.y)) |
                RegionBottom * (cursor.y >= (wpos.bottom - border.y));

            if(result != 0 && ImGui::IsAnyItemHovered()) break;

            switch(result) {
            case RegionLeft:
                return HTLEFT;
            case RegionRight:
                return HTRIGHT;
            case RegionTop:
                return HTTOP;
            case RegionBottom:
                return HTBOTTOM;
            case RegionTop | RegionLeft:
                return HTTOPLEFT;
            case RegionTop | RegionRight:
                return HTTOPRIGHT;
            case RegionBottom | RegionLeft:
                return HTBOTTOMLEFT;
            case RegionBottom | RegionRight:
                return HTBOTTOMRIGHT;
            case RegionClient:
            default:
                if(static_cast<float>(cursor.y - wpos.top) < scale(CAPTION_SIZE)) return HTCAPTION;
                break;
            }

            return HTCLIENT;
        }

        default:
            break;
        }

        return CallWindowProc(w->oldProc, hwnd, uMsg, wParam, lParam);
    }
} // namespace LRI::RCI
