#include "UI/window.h"

#define GLFW_EXPOSE_NATIVE_WIN32
#include "glfw/glfw3native.h"

#include "Dwmapi.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "windowsx.h"

#include <print>
#include "UI/style.h"

namespace LRI::RCI {
    LRESULT borderlessCallback(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
        Window* w = reinterpret_cast<Window*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
        if(!w) return DefWindowProc(hwnd, uMsg, wParam, lParam);

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

        case WM_NCHITTEST: {
            POINT cursor = {GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)};

            // Formatter goes berserk with this :(
            const POINT border{static_cast<LONG>(static_cast<float>(GetSystemMetrics(SM_CXFRAME) +
                                                                    GetSystemMetrics(SM_CXPADDEDBORDER)) *
                                                 style::getScaleFactor()),
                               static_cast<LONG>(static_cast<float>(GetSystemMetrics(SM_CYFRAME) +
                                                                    GetSystemMetrics(SM_CXPADDEDBORDER)) *
                                                 style::getScaleFactor())};


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
                if(w->inCaption(cursor.y - wpos.top)) return HTCAPTION;
                break;
            }

            return HTCLIENT;
        }

        default:
            break;
        }

        return CallWindowProc(w->oldProc, hwnd, uMsg, wParam, lParam);
    }

    Window::Window() : window(nullptr), hwnd(nullptr), oldProc(nullptr) {
        glfwDefaultWindowHints();
        glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);
        glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, GLFW_TRUE);
        glfwWindowHint(GLFW_MAXIMIZED, GLFW_TRUE);

        window = glfwCreateWindow(1280, 720, "Rocket Control Interface", nullptr, nullptr);
        hwnd = glfwGetWin32Window(window);

        glfwSetWindowOpacity(window, 1);
        glfwMakeContextCurrent(window);
        glfwSwapInterval(1);

        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();
        ImGui_ImplGlfw_InitForOpenGL(window, true);
        ImGui_ImplOpenGL3_Init();

        oldProc = reinterpret_cast<WNDPROC>(
            SetWindowLongPtr(hwnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(borderlessCallback)));
        SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));
        glfwSetWindowUserPointer(window, this);

        // Needed for rounded corners
        MARGINS m = {1, 1, 1, 1};
        DwmExtendFrameIntoClientArea(hwnd, &m);

        BOOL val = TRUE;
        DwmSetWindowAttribute(hwnd, DWMWA_USE_IMMERSIVE_DARK_MODE, &val, sizeof(val));

        glfwSetWindowRefreshCallback(window, [](auto* w) {
            reinterpret_cast<Window*>(glfwGetWindowUserPointer(w))->frame();
            DwmFlush();
        });

        LONG style = GetWindowLong(hwnd, GWL_STYLE);
        style |= WS_OVERLAPPEDWINDOW;
        style &= ~WS_POPUP;
        SetWindowLong(hwnd, GWL_STYLE, style);

        style::setWindowIcon(window);
        style::setupBirdIcon();
        font::setupFonts();

        {
            float mX, mY;
            GLFWmonitor* mon = glfwGetPrimaryMonitor();
            glfwGetMonitorContentScale(mon, &mX, &mY);
            style::setScaleFactor((mX + mY) / 2);
        }

        io.IniFilename = nullptr;
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    }

    void Window::frame() {
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        int wX, wY;
        glfwGetWindowSize(window, &wX, &wY);
        style::setFrameWindowSize({static_cast<float>(wX), static_cast<float>(wY)});

        ImGui::Text("hello");
        ImGui::GetBackgroundDrawList()->AddRectFilled({0, 0}, {style::getWindowSize().x, 50}, style::WHITE);

        ImGui::Render();
        glViewport(0, 0, wX, wY);
        glClearColor(0, 0, 0, 1);
        glClear(GL_COLOR_BUFFER_BIT);

        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);
    }

    bool Window::inCaption(LONG cursorY) { return cursorY < 50; }

    void Window::loop() {
        while(!glfwWindowShouldClose(window)) {
            glfwPollEvents();
            frame();
        }
    }

    Window::~Window() {
        style::cleanupBirdTex();
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
        glfwDestroyWindow(window);
    }

} // namespace LRI::RCI
