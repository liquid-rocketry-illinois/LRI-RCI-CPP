#include "UI/window.h"

#define GLFW_EXPOSE_NATIVE_WIN32
#include "glfw/glfw3native.h"

#include "Dwmapi.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "windowsx.h"

#include <future>
#include <print>

#include "RCP_Host/RCP_Host.h"
#include "UI/fontawesome.h"
#include "UI/style.h"
#include "UI/vscode_icons.h"
#include "VERSION.h"
#include "nfd.h"
#include "util/guards.h"
#include "util/settings.h"
#include "util/system.h"
#include "UI/TargetView.h"

#define CAPHEIGHT (40_sc)

namespace LRI::RCI {
    namespace {
        enum class PState : uint8_t {
            NONE,
            TARGET_EDIT,
            TEST_LOG,
            HARDWARE,
        } state = PState::NONE;

        bool insideCaption(LONG cursory) { return static_cast<float>(cursory) < CAPHEIGHT; }

        WindowInfo winfo{insideCaption, nullptr};
        GLFWwindow* window;
        bool firstLoop = true;

        std::string versionString;
        float verStringHeight = 0;
        Windowlet* openView = nullptr;

        enum class OpenType {
            NONE,
            TLOG,
            TARGET,
        } openType;

        std::optional<std::future<std::filesystem::path>> popupSelectedFile = std::nullopt;

        void renderChooserPopup(ImVec2 pos) {
            ImGui::SetNextWindowPos(pos);
            ImGui::SetNextWindowSize({200_sc, 300_sc});
            ImGui::PushStyleColor(ImGuiCol_Border, style::PURPLE);
            SCOPE_EXIT { ImGui::PopStyleColor(); };
            if(!ImGui::BeginPopup("##dropdownchooser", ImGuiWindowFlags_AlwaysVerticalScrollbar)) return;
            ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, {4_sc, 4_sc});

            SCOPE_EXIT {
                ImGui::PopStyleVar();
                ImGui::EndPopup();
            };

            // 200 window size - 8px padding on both sides - 14 scrollbar size
            ImVec2 buttonSize = {170_sc, 0};

            ImGui::Button(ICON_VS_ADD " New Target##addtarget", buttonSize);
            if(ImGui::Button(ICON_VS_FOLDER " Open Target##opentarget", buttonSize)) {
                popupSelectedFile = pickFile("target");
                openType = OpenType::TARGET;
            }

            if(ImGui::Button(ICON_VS_FOLDER " Open Testlog##opentestlog", buttonSize)) {
                popupSelectedFile = pickFile("testlog");
                openType = OpenType::TLOG;
            }

            ImGui::Button(ICON_VS_EDIT " Target Editor##edittarget", buttonSize);

            ImGui::Separator();

            if(settings::getRecents().empty()) {
                ImGui::BeginDisabled();
                ImGui::Text("No Recents");
                ImGui::EndDisabled();
                return;
            }

            int id = 0;
            buttonSize.y = 40_sc;
            for(const settings::Recent& r : settings::getRecents()) {
                ImGui::PushID(id++);

                ImVec2 cursor = ImGui::GetCursorPos();
                ImVec2 extraPopupPos = ImGui::GetCursorScreenPos() + ImVec2{172_sc, 0};

                ImGui::Button("##button", buttonSize);
                ImVec2 afterButton = ImGui::GetCursorPos();
                bool wasHovered = false;
                static bool alreadyOpen = false;
                static int coyoteFrames = 0;
                if(ImGui::IsItemHovered()) {
                    wasHovered = true;
                    if(!alreadyOpen) {
                        ImGui::OpenPopup("##dropdownextras", ImGuiPopupFlags_NoReopen);
                        coyoteFrames = 3;
                    }
                }

                ImGui::PushFont(nullptr, 28_sc);
                ImVec2 iconsize = ImGui::CalcTextSize(&r.display_char[0]);
                cursor += ImVec2{5_sc, (buttonSize.y - iconsize.y) / 2};
                ImGui::SetCursorPos(cursor);
                if(r.type == settings::RecentType::TARGET) ImGui::Text("%s", &r.display_char[0]);
                else ImGui::Text(ICON_VS_GRAPH_LINE);
                ImGui::PopFont();

                cursor.x += iconsize.x;
                ImGui::SetCursorPos(cursor);
                ImGui::Text("%s", r.displayName.c_str());

                ImGui::BeginDisabled();
                ImGui::PushFont(nullptr, 12_sc);
                cursor += ImVec2{0, 16_sc};
                ImGui::SetCursorPos(cursor);
                if(r.type == settings::RecentType::TARGET) ImGui::Text("%s", r.connectName.c_str());
                else ImGui::Text("Test Log");
                ImGui::PopFont();
                ImGui::EndDisabled();

                ImGui::SetNextWindowPos(extraPopupPos);
                if(ImGui::BeginPopup("##dropdownextras", ImGuiWindowFlags_ChildWindow)) {
                    alreadyOpen = true;
                    ImGui::Text("HELLO");
                    wasHovered = wasHovered || ImGui::IsWindowHovered();
                    if(!wasHovered) {
                        if(coyoteFrames-- <= 0) ImGui::CloseCurrentPopup();
                    }
                    ImGui::EndPopup();
                }
                else alreadyOpen = false;

                ImGui::SetCursorPos(afterButton);
                ImGui::PopID();
            }
            ImGui::Dummy(V0);
        }

        void renderCaption() {
            ImGui::SetNextWindowPos(V0);

            // Size of the caption
            const ImVec2 capsize = {style::getWindowSize().x, CAPHEIGHT};

            // Size of the largest square that fits into the caption. Used for buttons
            const ImVec2 csquare = {capsize.y, capsize.y};

            const float buttonHeight = CAPHEIGHT - 10_sc;
            const float buttonY = (CAPHEIGHT - buttonHeight) / 2;
            const ImVec2 bsquare = {buttonHeight, buttonHeight};

            constexpr ImGuiWindowFlags FLAGS =
                ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoDocking;

            ImGui::SetNextWindowSize(capsize);
            font::pushCodicons();
            SCOPE_EXIT {
                ImGui::PopFont();
                ImGui::End();
            };

            if(!ImGui::Begin("##caption", nullptr, FLAGS)) return;

            // Draw little bird icon in top left corner
            ImGui::SetCursorPos(V0);
            ImGui::Image(style::getBirdTex(), csquare);

            ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 8_sc);
            ImGui::PushStyleVar(ImGuiStyleVar_ButtonTextAlign, {0, 0.5f});

            // Draw hamborger
            ImGui::SetCursorPos({csquare.x + 5_sc, buttonY});
            ImGui::Button(ICON_VS_THREE_BARS "##hamborger", bsquare);

            // Draw the target dropdown
            const bool disableDueToFilePicker = popupSelectedFile.has_value();
            if(disableDueToFilePicker) {
                ImGui::BeginDisabled();
                ImGui::OpenPopup("##dropdownchooser", ImGuiPopupFlags_NoReopen);
            }
            bool dchooserOpen = ImGui::IsPopupOpen("##dropdownchooser");
            if(dchooserOpen) ImGui::PushStyleColor(ImGuiCol_Button, style::BUTTONHOVER);
            ImGui::SetCursorPos({csquare.x + 5_sc + bsquare.x + 5_sc, buttonY});
            if(ImGui::Button(" " ICON_VS_ROCKET " No Target " ICON_VS_CHEVRON_DOWN " ##popup", {0, bsquare.y})) {
                ImGui::OpenPopup("##dropdownchooser");
            }
            if(dchooserOpen) ImGui::PopStyleColor();
            renderChooserPopup({csquare.x + 5_sc + bsquare.x + 5_sc, capsize.y - buttonY});
            ImGui::PopStyleVar(2);
            if(disableDueToFilePicker) ImGui::EndDisabled();

            // Draw window control buttons
            // Minimize button
            ImVec2 cpos = {capsize.x - (3 * csquare.x), 0};
            ImGui::SetCursorPos(cpos);
            if(ImGui::Button(ICON_VS_CHROME_MINIMIZE "##minimize", csquare)) glfwIconifyWindow(window);

            // Maximize/restore
            cpos.x += csquare.y;
            ImGui::SetCursorPos(cpos);
            if(glfwGetWindowAttrib(window, GLFW_MAXIMIZED)) {
                if(ImGui::Button(ICON_VS_CHROME_RESTORE "##restore", csquare)) glfwRestoreWindow(window);
            }
            else {
                if(ImGui::Button(ICON_VS_CHROME_MAXIMIZE "##restore", csquare)) glfwMaximizeWindow(window);
            }

            // Close button
            cpos.x += csquare.y;
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, style::RED);
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, style::LRED);
            ImGui::SetCursorPos(cpos);
            if(ImGui::Button(ICON_VS_CHROME_CLOSE "##close", csquare)) glfwSetWindowShouldClose(window, true);
            ImGui::PopStyleColor(2);
        }

        void renderConfigModal() {
            if(!settings::hadParseError()) return;
            if(firstLoop) ImGui::OpenPopup("Config Error##cfgerror");

            ImVec2 center = ImGui::GetMainViewport()->GetCenter();
            ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, {0.5, 0.5});
            if(!ImGui::BeginPopupModal("Config Error##cfgerror", nullptr,
                                       ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove))
                return;
            SCOPE_EXIT { ImGui::EndPopup(); };

            ImGui::Text("Error loading config:\n%s", settings::getErrorString().c_str());
            ImGui::Text("Load New Config: ");
            ImGui::SameLine();
            if(ImGui::Button("OK")) {
                settings::loadFreshConfig();
                ImGui::CloseCurrentPopup();
            }
        }

        void frame() {
            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();

            int wX, wY;
            glfwGetWindowSize(window, &wX, &wY);
            style::setFrameWindowSize({static_cast<float>(wX), static_cast<float>(wY)});

            ImGui::ShowDemoWindow();
            renderCaption();
            renderConfigModal();

            if(openView != nullptr) openView->render();

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

            ImGui::GetBackgroundDrawList()->AddText({10_sc, static_cast<float>(wY) - verStringHeight - 10_sc},
                                                    style::GRAY_SEMITRANSPARENTi, versionString.c_str());

            ImGui::Render();
            glViewport(0, 0, wX, wY);
            glClearColor(0, 0, 0, 1);
            glClear(GL_COLOR_BUFFER_BIT);

            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
            glfwSwapBuffers(window);
        }
    } // namespace

    LRESULT borderlessCallback(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
        auto* i = reinterpret_cast<WindowInfo*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
        if(i == nullptr) return DefWindowProc(hwnd, uMsg, wParam, lParam);

        switch(uMsg) {
        case WM_NCACTIVATE:
            return DefWindowProc(hwnd, uMsg, wParam, -1);

        case WM_NCPAINT:
            return DefWindowProc(hwnd, uMsg, wParam, lParam);

        case WM_NCCALCSIZE: {
            RECT& rect = *reinterpret_cast<RECT*>(lParam);
            RECT client = rect;

            CallWindowProc(i->oldProc, hwnd, uMsg, wParam, lParam);

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
                if(i->cfunc(cursor.y - wpos.top)) return HTCAPTION;
                break;
            }

            return HTCLIENT;
        }

        default:
            break;
        }

        return CallWindowProc(i->oldProc, hwnd, uMsg, wParam, lParam);
    }

    GLFWwindow* setupBorderlessWindow(WindowInfo* usrptr) {
        glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);
        glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, GLFW_TRUE);
        glfwWindowHint(GLFW_MAXIMIZED, GLFW_TRUE);

        GLFWwindow* win = glfwCreateWindow(1280, 720, "RCI", nullptr, nullptr);
        HWND hwnd = glfwGetWin32Window(win);

        // Needed for rounded corners
        MARGINS m = {1, 1, 1, 1};
        DwmExtendFrameIntoClientArea(hwnd, &m);

        BOOL val = TRUE;
        DwmSetWindowAttribute(hwnd, DWMWA_USE_IMMERSIVE_DARK_MODE, &val, sizeof(val));

        LONG style = GetWindowLong(hwnd, GWL_STYLE);
        style |= WS_OVERLAPPEDWINDOW;
        style &= ~WS_POPUP;
        SetWindowLong(hwnd, GWL_STYLE, style);

        SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(usrptr));
        usrptr->oldProc = reinterpret_cast<WNDPROC>(
            SetWindowLongPtr(hwnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(borderlessCallback)));

        return win;
    }

    void show() {
        glfwDefaultWindowHints();
        glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
        window = setupBorderlessWindow(&winfo);

        glfwSetWindowOpacity(window, 1);
        glfwMakeContextCurrent(window);
        glfwSwapInterval(1);

        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();
        ImGui_ImplGlfw_InitForOpenGL(window, true);
        ImGui_ImplOpenGL3_Init();

        glfwSetWindowRefreshCallback(window, [](auto* w) {
            frame();
            DwmFlush();
        });

        style::setWindowIcon(window);
        style::setupBirdIcon();
        font::setupFonts();
        style::setImGuiStyles();

        {
            float mX, mY;
            GLFWmonitor* mon = glfwGetPrimaryMonitor();
            glfwGetMonitorContentScale(mon, &mX, &mY);
            style::setScaleFactor((mX + mY) / 2);
        }

        io.IniFilename = nullptr;
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

        font::pushAwesome();
        versionString =
            "RCI " + std::string(RCI_VERSION, RCI_VERSION_END) + "\nRCP " + std::string(RCP_VERSION, RCP_VERSION_END);
        verStringHeight = ImGui::CalcTextSize(versionString.c_str()).y;
        ImGui::PopFont();
        glfwSetWindowSizeLimits(window, static_cast<int>(400_sc), static_cast<int>(300_sc), GLFW_DONT_CARE,
                                GLFW_DONT_CARE);

        glfwShowWindow(window);
        firstLoop = true;

        while(!glfwWindowShouldClose(window)) {
            glfwPollEvents();
            frame();
            firstLoop = false;
            if(openView != nullptr && openView->shouldClose()) {
                delete openView;
                openView = nullptr;
            }
            if(popupSelectedFile) {
                if(popupSelectedFile->wait_for(std::chrono::milliseconds(0)) == std::future_status::ready) {
                    if(openType == OpenType::TARGET) openView = new TargetView(popupSelectedFile->get());

                    popupSelectedFile = std::nullopt;
                    openType = OpenType::NONE;
                }
            }
        }

        style::cleanupBirdTex();
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
        glfwDestroyWindow(window);
    }

} // namespace LRI::RCI
