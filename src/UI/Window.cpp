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
    Window::Window() : window(nullptr), oldProc(nullptr), classid(0) {
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

        ImGui::StyleColorsDark();

        ImGui_ImplGlfw_InitForOpenGL(window, true);
        ImGui_ImplOpenGL3_Init();

        glfwGetWindowContentScale(window, &style::scaling_factor, nullptr);
        glfwSetWindowSizeLimits(window, static_cast<int>(400_sc), static_cast<int>(300_sc), GLFW_DONT_CARE,
                                GLFW_DONT_CARE);
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

            ImGui::GetBackgroundDrawList()->AddRectFilled(
                ImGui::GetMainViewport()->Pos, ImGui::GetMainViewport()->Pos + ImVec2(ImGui::GetMainViewport()->Size.x, scale(40)),
                0xFFFFFFFF);

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
