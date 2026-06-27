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
#include "UI/vscode_icons.h"
#include "util/guards.h"

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

        window = glfwCreateWindow(1280, 720, "RCI", nullptr, nullptr);
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

        style::setImGuiStyles();
    }

    void Window::frame() {
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        int wX, wY;
        glfwGetWindowSize(window, &wX, &wY);
        style::setFrameWindowSize({static_cast<float>(wX), static_cast<float>(wY)});

        {
            ImVec2 blankSpace = style::getWindowSize() - ImVec2{0, 40_sc};
            ImVec2 impos;
            ImVec2 imsize;

            if(blankSpace.y < blankSpace.x) {
                float leftoverX = blankSpace.x - blankSpace.y;
                impos = {leftoverX / 2, 40_sc};
                imsize = {blankSpace.y, blankSpace.y};
            }

            else {
                float leftoverY = blankSpace.y - blankSpace.x;
                impos = {0, 40_sc + (leftoverY / 2)};
                imsize = {blankSpace.x, blankSpace.x};
            }

            ImGui::GetBackgroundDrawList()->AddImage(style::getBirdTex(), impos, impos + imsize);
        }

        ImGui::ShowDemoWindow();
        renderCaption();
        for(Windowlet* w : windowlets) w->render();

        ImGui::Render();
        glViewport(0, 0, wX, wY);
        glClearColor(0, 0, 0, 1);
        glClear(GL_COLOR_BUFFER_BIT);

        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);
    }

    void Window::renderCaption() {
        ImGui::SetNextWindowPos(V0);

        // Size of the caption
        const ImVec2 capsize = {style::getWindowSize().x, 40_sc};

        // Size of the largest square that fits into the caption. Used for buttons
        const ImVec2 csquare = {40_sc, 40_sc};

        constexpr ImGuiWindowFlags FLAGS =
            ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoDocking;

        ImGui::SetNextWindowSize(capsize);
        if(!ImGui::Begin("##caption", nullptr, FLAGS)) return;
        font::pushCodicons();

        SCOPE_EXIT {
            ImGui::PopFont();
            ImGui::End();
        };

        // Draw little bird icon in top left corner
        ImGui::GetWindowDrawList()->AddRectFilled({0, 0}, capsize, style::DGRAYi);
        ImGui::SetCursorPos(V0);
        ImGui::Image(style::getBirdTex(), csquare);

        // Draw hamborger button
        ImGui::SetCursorPos({45_sc, 0});
        ImGui::Button(ICON_VS_THREE_BARS "##hamborger", csquare);

        // Draw window control buttons
        // Minimize button
        ImVec2 cpos = {style::getWindowSize().x - 120_sc, 0};
        ImGui::SetCursorPos(cpos);
        if(ImGui::Button(ICON_VS_CHROME_MINIMIZE "##minimize", csquare)) glfwIconifyWindow(window);

        // Maximize/restore
        cpos.x += 40_sc;
        ImGui::SetCursorPos(cpos);
        if(glfwGetWindowAttrib(window, GLFW_MAXIMIZED)) {
            if(ImGui::Button(ICON_VS_CHROME_RESTORE "##restore", csquare)) glfwRestoreWindow(window);
        }
        else {
            if(ImGui::Button(ICON_VS_CHROME_MAXIMIZE "##restore", csquare)) glfwMaximizeWindow(window);
        }

        // Close button
        cpos.x += 40_sc;
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, style::RED);
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, style::LRED);
        ImGui::SetCursorPos(cpos);
        if(ImGui::Button(ICON_VS_CHROME_CLOSE "##close", csquare)) glfwSetWindowShouldClose(window, true);
        ImGui::PopStyleColor(2);

        ImGui::SetCursorPos({90_sc, 0});
        // const char* targettext = "Open Target"
        ImGui::Button("E" ICON_VS_ACCOUNT "##popup", csquare);
    }

    bool Window::inCaption(LONG cursory) const { return static_cast<float>(cursory) < 40_sc; }

    void Window::loop() {
        while(!glfwWindowShouldClose(window)) {
            glfwPollEvents();
            frame();
        }
    }

    void Window::registerWindowlet(Windowlet* w) { windowlets.emplace(w); }

    Window::~Window() {
        style::cleanupBirdTex();
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
        glfwDestroyWindow(window);
    }
} // namespace LRI::RCI
