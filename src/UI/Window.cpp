#include "UI/Window.h"

#include <dwmapi.h>
#include <ranges>

#define GLFW_EXPOSE_NATIVE_WIN32
#include "glfw/glfw3native.h"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "implot.h"
#include "improgress.h"

#include "nlohmann/json.hpp"

#include "UI/Windowlet.h"
#include "UI/gutils.h"
#include "UI/vscode_icons.h"

#include "hardware/hwctrl.h"
#include "hardware/json.h"

#include "UI/AbridgedSensorViewer.h"
#include "UI/AngledActuatorViewer.h"
#include "UI/BoolSensorViewer.h"
#include "UI/DiscreteActuatorViewer.h"
#include "UI/EStopViewer.h"
#include "UI/ErrorWindow.h"
#include "UI/MotorViewer.h"
#include "UI/MultiSensorViewer.h"
#include "UI/PromptViewer.h"
#include "UI/SensorViewer.h"
#include "UI/StepperViewer.h"
#include "UI/TargetLogViewer.h"
#include "UI/TestStateViewer.h"

// Some helpers for setting up WModules
namespace {
    using namespace LRI::RCI;

    WModule* setupT6Window(const std::set<HardwareQualifier>& quals, const std::string wtitle, int wmtype,
                           const TargetTable6& t6) {
        auto devclass = static_cast<RCP_DeviceClass>(wmtype);
        std::set<uint8_t> validIds;
        for(const auto& id : t6.ids) {
            HardwareQualifier temp = {devclass, id};
            if(quals.contains(temp)) validIds.insert(id);
            else {
                hwctrl::addError(Error::LWARNING,
                                 "Tried to query for nonexistent hardware device {} in window {}, module type {}", temp,
                                 wtitle, wmtype);
            }
        }

        const auto qualSet = quals | std::views::filter([&validIds, devclass](const HardwareQualifier& q) {
                                 return q.devclass == devclass && validIds.contains(q.id);
                             }) |
            std::ranges::to<std::set>(); // All are an implicit hardware channel with channel 0

        switch(wmtype) {
        case RCP_DEVCLASS_MOTOR:
            return new MotorViewer(qualSet, t6.refresh);

        case RCP_DEVCLASS_DISCRETE_ACTUATOR:
            return new DiscreteActuatorViewer(qualSet, t6.refresh);

        case RCP_DEVCLASS_BOOL_SENSOR: {
            auto asChannels = qualSet |
                std::views::transform([](const HardwareQualifier& qual) { return HardwareChannel{qual, 0}; }) |
                std::ranges::to<std::set>();
            return new BoolSensorViewer(asChannels, t6.refresh);
        }

        case RCP_DEVCLASS_STEPPER:
            return new StepperViewer(qualSet, t6.refresh);

        case RCP_DEVCLASS_ANGLED_ACTUATOR:
            return new AngledActuatorViewer(qualSet, t6.refresh);

        default:
            return nullptr;
        }
    }

    WModule* setupClassicSensors(const std::set<HardwareQualifier>& quals, const std::string& wtitle, int wmtype,
                                 const TargetTable7& t7) {
        std::vector<HardwareQualifier> qualSet;

        for(const auto& qgroup : t7.ids) {
            for(const auto& id : qgroup.ids) {
                HardwareQualifier temp = {qgroup.devclass, id};
                auto ret = quals.find(temp);
                if(ret == quals.end())
                    hwctrl::addError(Error::LWARNING,
                                     "Tried to query for nonexistent hardware device {} in window {}, "
                                     "module type {}",
                                     temp, wtitle, wmtype);
                else qualSet.push_back(*ret);
            }
        }

        return new SensorViewer(qualSet, t7.classicShowControls);
    }

    WModule* setupAbridgedSensors(const std::set<HardwareQualifier>& quals, const std::string& wtitle, int wmtype,
                                  const TargetTable7& t7) {
        std::vector<std::vector<HardwareChannel>> channels;
        for(const auto& qgroup : t7.ids) {
            std::vector<HardwareChannel> channelLine;
            for(size_t i = 0; i < qgroup.ids.size(); i++) {
                HardwareChannel temp = {qgroup.devclass, qgroup.ids[i], qgroup.channels[i]};
                auto ret = quals.find(temp);
                if(ret == quals.end()) {
                    hwctrl::addError(Error::LWARNING,
                                     "Tried to query for nonexistent hardware device {} in window {}, module type {}",
                                     temp, wtitle, wmtype);
                }
                else channelLine.emplace_back(*ret, temp.channel);
            }

            channels.emplace_back(std::move(channelLine));
        }

        return new AbridgedSensorViewer(channels);
    }

    WModule* setupMultiSensors(const std::set<HardwareQualifier>& quals, const std::string& wtitle, int wmtype,
                               const TargetTable7& t7) {
        std::vector<MultiSensorViewer::GraphData> gd;

        for(const auto& qgroup : t7.ids) {
            MultiSensorViewer::GraphData graph;
            graph.title = qgroup.multiTitle;
            uint8_t channel = qgroup.channels[0];

            for(const auto& id : qgroup.ids) {
                HardwareQualifier temp = {qgroup.devclass, id};
                auto ret = quals.find(temp);
                if(ret == quals.end()) {
                    hwctrl::addError(Error::LWARNING,
                                     "Tried to query for nonexistent hardware device {} in window {}, module type {}",
                                     temp, wtitle, wmtype);
                }
                else graph.channels.emplace_back(*ret, channel);
            }

            gd.emplace_back(std::move(graph));
        }

        return new MultiSensorViewer(gd);
    }
} // namespace

namespace LRI::RCI {
    Window::Window() :
        window(nullptr), oldProc(nullptr), chooser(this), open(false), debugChordPrev(false), showDebug(false) {
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
        windowlets.insert(&chooser);
    }

    Window::~Window() {
        if(hwctrl::isOpen()) {
            hwctrl::end();
            auto ipath = getRoamingFolder() / "targets" / cfname;
            ImGui::SaveIniSettingsToDisk(ipath.string().c_str());

            for(auto* w : windowlets) delete w;
            windowlets.clear();
        }

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

#ifdef RCIDEBUG
            renderDebug();
#endif

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

    void Window::renderMenuPopup(ImVec2 poppos) {
        ImGui::SetNextWindowPos(poppos);
        if(ImGui::BeginPopup("menupopup", ImGuiWindowFlags_NoMove)) {
            if(ImGui::Button("Open Exports")) {
                openExportsFolder();
                ImGui::CloseCurrentPopup();
            }

            if(!hwctrl::isOpen()) ImGui::BeginDisabled();
            if(ImGui::Button("Reset Window Layout")) {
                preframe([this] {
                    auto ipath = std::filesystem::path("targets") / cfname;
                    ImGui::LoadIniSettingsFromDisk(ipath.string().c_str());
                });
                ImGui::CloseCurrentPopup();
            }
            if(!hwctrl::isOpen()) ImGui::EndDisabled();

            ImGui::EndPopup();
        }
    }

    void Window::renderTitlebar() {
        constexpr ImGuiWindowFlags CAPTION_FLAGS =
            ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoDocking;
        const ImVec2 vPos = ImGui::GetMainViewport()->Pos;
        const ImVec2 vSize = ImGui::GetMainViewport()->Size;
        const float capheight = scale(CAPTION_SIZE);
        const ImVec2 capsize = ImVec2{vSize.x, capheight};
        const ImVec2 maxSquare = ImVec2{capheight, capheight};
        const float textY = (capheight - 16_sc) / 2;
        const ImVec2 smallButtonSize = maxSquare * 0.75;
        const float buttonY = (capheight - smallButtonSize.x) / 2;

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

        ImGui::SameLine(0, 10_sc);
        ImGui::SetCursorPosY(buttonY);
        ImGui::PushStyleColor(ImGuiCol_Button, colors::CTRANSPARENT);
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, colors::LOW_SEMITRANSPARENT);
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, colors::HIGH_SEMITRANSPARENT);
        ImGui::PushFont(nullptr, 20); // Little bigger font so the button text doesnt look goofy
        float prevX = ImGui::GetCursorPosX();
        if(ImGui::Button(ICON_VS_MENU "##menupop", smallButtonSize)) ImGui::OpenPopup("menupopup");
        ImGui::PopFont();
        renderMenuPopup({prevX, capheight});
        ImGui::PopStyleColor(3); // Pop after the popup so the buttons inside are also semitransparent

        ImGui::SameLine(0, 10_sc);
        ImGui::SetCursorPosY(textY);

        if(hwctrl::isOpen()) {
            ImGui::Text("%s | %s | Packet Buffer Size: %d | Polling Rate: ", openTarget.c_str(), openInterf.c_str(), 0);
            ImGui::SameLine(0, 5);
            ImGui::SetNextItemWidth(40_sc);
            ImGui::SetCursorPosY(textY);
            ImGui::InputInt("##pollrate", &hwctrl::POLLS_PER_UPDATE, 0);
            if(hwctrl::POLLS_PER_UPDATE < 1) hwctrl::POLLS_PER_UPDATE = 1;

            ImGui::SameLine();
            ImGui::SetCursorPosY(textY);

            ImGui::Text(" | ");

            ImGui::SameLine();
            ImGui::SetCursorPosY(textY);

            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, colors::BUTTON_CLOSE_HOVERED);
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, colors::BUTTON_CLOSE_ACTIVE);

            if(ImGui::TimedButton("CLOSE", closeTimer)) {
                ImGui::SameLine();
                ImGui::SetCursorPosY(textY);
                ImGui::CircleProgressBar("##clearallprogressspinner", 10, 3, colors::TEXTi,
                                         closeTimer.timeSince() / WModule::CONFIRM_HOLD_TIME);

                if(closeTimer.timeSince() > WModule::CONFIRM_HOLD_TIME) {
                    preframe([&] {
                        auto ipath = getRoamingFolder() / "targets" / cfname;
                        ImGui::SaveIniSettingsToDisk(ipath.string().c_str());

                        hwctrl::end();
                        for(auto* w : windowlets) delete w;
                        windowlets.clear();
                        windowlets.insert(&chooser);
                    });
                }
            }

            ImGui::PopStyleColor(2);

            hwctrl::update();
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
        if(hwctrl::isOpen()) ImGui::BeginDisabled();
        if(ImGui::Button(ICON_VS_CHROME_CLOSE "##close", maxSquare)) glfwSetWindowShouldClose(window, GLFW_TRUE);
        if(hwctrl::isOpen()) ImGui::EndDisabled();

        ImGui::PopStyleColor(5);
    }

    void Window::renderDebug() {
        bool chord = ImGui::GetIO().KeyCtrl && ImGui::GetIO().KeyAlt && ImGui::IsKeyDown(ImGuiKey_D);
        if(debugChordPrev != chord) {
            debugChordPrev = chord;
            if(chord) {
                showDebug = !showDebug;
            }
        }

        if(showDebug) {
            ImGui::ShowDemoWindow();
            ImPlot::ShowDemoWindow();
        }
    }

    void Window::preframe(std::function<void()> func) { preframes.emplace_back(std::move(func)); }

    void Window::startTarget(RCP_Interface* interf, const TargetConfig& config, const std::string& confName) {
        cfname = confName + ".ini";
        openTarget = config.name;
        openInterf = interf->interfaceType();
        hwctrl::start(interf, config);

        const std::set<HardwareQualifier>& quals = hwctrl::getQuals();
        std::vector<Windowlet*> wls;

        for(const auto& [title, modules] : config.windows) {
            std::vector<WModule*> wms;

            for(const auto& [type, t6, t7] : modules) {
                switch(type) {
                case -2:
                    wms.push_back(new ErrorWindow());
                    break;

                case -1:
                    wms.push_back(new EStopViewer());
                    break;

                case RCP_DEVCLASS_TEST_STATE:
                    wms.push_back(new TestStateViewer());
                    break;

                case RCP_DEVCLASS_MOTOR:
                case RCP_DEVCLASS_DISCRETE_ACTUATOR:
                case RCP_DEVCLASS_BOOL_SENSOR:
                case RCP_DEVCLASS_STEPPER:
                case RCP_DEVCLASS_ANGLED_ACTUATOR: {
                    auto* wm = setupT6Window(quals, title, type, t6);
                    if(wm != nullptr) wms.push_back(wm);
                    break;
                }

                case RCP_DEVCLASS_PROMPT:
                    wms.push_back(new PromptViewer());
                    break;

                case RCP_DEVCLASS_TARGET_LOG:
                    wms.push_back(new TargetLogViewer());
                    break;

                case RCP_DEVCLASS_AM_PRESSURE:
                case RCP_DEVCLASS_TEMPERATURE:
                case RCP_DEVCLASS_PRESSURE_TRANSDUCER:
                case RCP_DEVCLASS_RELATIVE_HYGROMETER:
                case RCP_DEVCLASS_LOAD_CELL:
                case RCP_DEVCLASS_FLOW_METER:
                case RCP_DEVCLASS_ALTITUDE:
                case RCP_DEVCLASS_RADIO_STRENGTH:
                case RCP_DEVCLASS_POWERMON:
                case RCP_DEVCLASS_ACCELEROMETER:
                case RCP_DEVCLASS_GYROSCOPE:
                case RCP_DEVCLASS_MAGNETOMETER:
                case RCP_DEVCLASS_RPY:
                case RCP_DEVCLASS_GPS:
                case RCP_DEVCLASS_QUATERNION: {
                    WModule* wm = nullptr;

                    if(t7.mode == SensorViewerMode::CLASSIC) wm = setupClassicSensors(quals, title, type, t7);
                    else if(t7.mode == SensorViewerMode::ABRIDGED) wm = setupAbridgedSensors(quals, title, type, t7);
                    else if(t7.mode == SensorViewerMode::MULTI) wm = setupMultiSensors(quals, title, type, t7);

                    if(wm != nullptr) wms.push_back(wm);
                    break;
                }

                default:
                    hwctrl::addError(Error::LWARNING, "Unknown WModule type {} in window {}", type, title);
                    break;
                }
            }

            wls.push_back(new Windowlet(title, std::move(wms)));
        }

        preframe([this, wls] {
            auto ipath = getRoamingFolder() / "targets" / cfname;
            ImGui::LoadIniSettingsFromDisk(ipath.string().c_str());
            windowlets.clear();
            windowlets.insert(wls.cbegin(), wls.cend());
        });
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
