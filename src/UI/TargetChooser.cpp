#include "UI/TargetChooser.h"

#include <filesystem>
#include <optional>

#include "improgress.h"
#include "nlohmann/json.hpp"

#include "hardware/json.h"

#include "interfaces/COMPort.h"
#include "interfaces/RCP_Interface.h"
#include "interfaces/TCPSocket.h"
#include "interfaces/VirtualPort.h"

#include "UI/Window.h"
#include "UI/gutils.h"
#include "UI/vscode_icons.h"
#include "hardware/hwctrl.h"

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

// Interface Choosing
namespace {
    using namespace LRI::RCI;

    class InterfaceChooser {
        static int CLASSID;

    protected:
        const int classid;
        bool lockDropdown;

    public:
        InterfaceChooser() : classid(CLASSID++), lockDropdown(false) {}
        virtual ~InterfaceChooser() = default;

        virtual void render() = 0;
        virtual bool shouldLockDropdown() { return lockDropdown; }
        virtual void startConnection() = 0;
        virtual RCP_Interface* getInterfAndCleanup() = 0;
    };

    int InterfaceChooser::CLASSID = 0;

    class COMPortChooser : public InterfaceChooser {
        size_t selectedPort;
        bool error;
        int baud;
        bool arduinoMode;
        COMPort* port;

    public:
        COMPortChooser() : selectedPort(0), error(false), baud(115200), arduinoMode(false), port(nullptr) {}
        ~COMPortChooser() override = default;

        void render() override {
            ImGui::PushID("COMPortChooser");
            ImGui::PushID(classid);
            SCOPE_EXIT {
                ImGui::PopID();
                ImGui::PopID();
            };

            const bool disable = port != nullptr;

            if(disable) ImGui::BeginDisabled();

            ImGui::Text("Choose Serial Port: ");
            ImGui::SameLine();
            const auto& serialDevs = getSerialDevs();
            if(serialDevs.empty()) ImGui::Text("No Ports Detected");
            else if(ImGui::BeginCombo("##portselectcombo", serialDevs[selectedPort].second.c_str())) {
                for(size_t i = 0; i < serialDevs.size(); i++) {
                    ImGui::PushID(static_cast<int>(i));
                    bool selected = i == selectedPort;
                    if(ImGui::Selectable((serialDevs[i]).second.c_str(), &selected)) selectedPort = i;
                    if(selected) ImGui::SetItemDefaultFocus();
                    ImGui::PopID();
                }

                ImGui::EndCombo();
            }

            ImGui::SameLine();
            ImGui::PushStyleColor(ImGuiCol_Button, colors::CTRANSPARENT);
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, colors::LOW_SEMITRANSPARENT);
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, colors::HIGH_SEMITRANSPARENT);
            if(ImGui::Button(ICON_VS_REFRESH "##refreshserials")) {
                selectedPort = 0;
                enumSerialDevs();
            }
            ImGui::PopStyleColor(3);

            // Input for baud rate
            ImGui::Text("Baud Rate: ");
            ImGui::SameLine();
            ImGui::SetNextItemWidth(100_sc);
            ImGui::InputInt("##comportchooserbaudinput", &baud);
            if(baud < 0) baud = 0;
            else if(baud > 500'000) baud = 500'000;

            ImGui::SameLine();
            ImGui::Text(" | Arduino Mode: ");
            ImGui::SameLine();
            ImGui::Checkbox("##arduinomode", &arduinoMode);

            if(disable) ImGui::EndDisabled();

            if(port == nullptr) return;

            // If the port allocated but did not open then show an error
            if(port->didPortOpenFail()) {
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1, 0, 0, 1));
                ImGui::Text("Error Connecting to Serial Port stage %lu code %lu)", port->lastError().code,
                            port->lastError().stage);
                ImGui::PopStyleColor();

                ImGui::SameLine();
                if(ImGui::Button("OK##comportchoosererror")) {
                    lockDropdown = false;
                    delete port;
                    port = nullptr;
                }
            }

            // While the port is readying, don't return it just yet and display a loading spinner
            else if(!port->isOpen()) {
                ImGui::SameLine();
                ImGui::Spinner("##comportchooserspinner", 8_sc, 1, colors::REBECCAi);
            }
        }

        void startConnection() override {
            // If connect, then create the COMPort
            lockDropdown = true;
            port = new COMPort(std::move(R"(\\.\)" + getSerialDevs()[selectedPort].first), baud, arduinoMode);
        }

        RCP_Interface* getInterfAndCleanup() override {
            if(port != nullptr && port->isOpen()) {
                auto* temp = port;
                port = nullptr;
                lockDropdown = false;
                return temp;
            }

            return nullptr;
        }
    };

    class TCPInterfaceChooser : public InterfaceChooser {
        int ip[4] = {192, 168, 1, 4};
        int port;
        bool server;

        TCPSocket* interf;

    public:
        TCPInterfaceChooser() : port(5000), server(false), interf(nullptr) {}
        ~TCPInterfaceChooser() override = default;

        void render() override {
            ImGui::PushID("TCPSocketChooser");
            ImGui::PushID(classid);
            SCOPE_EXIT {
                ImGui::PopID();
                ImGui::PopID();
            };

            bool isnull = interf == nullptr;
            if(!isnull) ImGui::BeginDisabled();
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0, 1, 0, 1));
            ImGui::Text("Make sure to set the computer's static IP in control panel!");
            ImGui::PopStyleColor();

            // Buttons to pick between client or server
            bool tempserver = server;
            if(tempserver) ImGui::BeginDisabled();
            if(ImGui::Button("Server")) server = true;
            if(tempserver) ImGui::EndDisabled();

            ImGui::SameLine();
            if(!tempserver) ImGui::BeginDisabled();
            if(ImGui::Button("Client")) server = false;
            if(!tempserver) ImGui::EndDisabled();

            // If in client mode, ask for ip address
            if(!tempserver) {
                ImGui::Text("Server Address: ");
                ImGui::SameLine();
                ImGui::SetNextItemWidth(200_sc);
                ImGui::InputInt4("##serveraddrinput", ip);
                for(int& i : ip) {
                    if(i < 0) i = 0;
                    else if(i > 255) i = 255;
                }
            }

            // Port input
            ImGui::Text("Port: ");
            ImGui::SameLine();
            ImGui::SetNextItemWidth(48_sc);
            ImGui::InputInt("##portinput", &port, 0);
            if(port < 0) port = 0;
            else if(port > 65535) port = 65535;

            // If the interface is null, keep rendering
            if(interf == nullptr) return;

            // If the interface has been created but did not open properly, display error and continue rendering
            if(interf->didPortOpenFail()) {
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1, 0, 0, 1));
                TCPSocket::Error error = interf->lastError();
                ImGui::Text("Error opening TCP socket: stage %lu, code %lu", error.stage, error.code);
                ImGui::PopStyleColor();

                ImGui::SameLine();
                if(ImGui::Button("OK")) {
                    lockDropdown = false;
                    delete interf;
                    interf = nullptr;
                }

                return;
            }

            // While the interface is open but not ready, keep waiting for the connection to be established
            if(!interf->isOpen()) {
                ImGui::SameLine();
                ImGui::Text("Waiting for connection");
                ImGui::SameLine();
                ImGui::Spinner("##tcpwaitspinner", 8, 1, colors::REBECCAi);

                if(ImGui::Button("Cancel")) {
                    lockDropdown = false;
                    delete interf;
                    interf = nullptr;
                }
            }
        }

        RCP_Interface* getInterfAndCleanup() override {
            if(interf != nullptr && interf->isOpen()) {
                // Once the interface is ready to go, return it to the TargetChooser
                lockDropdown = false;
                auto* temp = interf;
                interf = nullptr; // Return port but clear internal state so the class instance can be reused
                return temp;
            }

            return nullptr;
        }

        void startConnection() override {
            lockDropdown = true;
            interf =
                new TCPSocket(port, server ? sf::IpAddress(0, 0, 0, 0) : sf::IpAddress(ip[0], ip[1], ip[2], ip[3]));
        }
    };

    class VirtualPortChooser : public InterfaceChooser {
        VirtualPort* port = nullptr;

    public:
        VirtualPortChooser() = default;
        ~VirtualPortChooser() override = default;

        void render() override {}

        void startConnection() override {
            port = new VirtualPort();
            lockDropdown = true;
        }

        RCP_Interface* getInterfAndCleanup() override {
            if(port != nullptr) {
                lockDropdown = false;
                auto* temp = port;
                port = nullptr;
                return temp;
            }

            return nullptr;
        }
    };
} // namespace

namespace {
    using namespace LRI::RCI;

    class TargetWTF : public WindowTitlebarFunctions {
        const std::string targetName;
        const std::string interfName;
        const std::string userIni;
        const std::string canonIni;
        Window* const owner;
        StopWatch closeTimer;

    public:
        TargetWTF(std::string targetName, std::string interfName, std::string userIni, std::string canonIni,
                  Window* owner) :
            targetName(std::move(targetName)), interfName(std::move(interfName)), userIni(std::move(userIni)),
            canonIni(std::move(canonIni)), owner(owner) {}
        ~TargetWTF() override {
            ImGui::SaveIniSettingsToDisk(userIni.c_str());
            hwctrl::end();
            allowScreenTurnoff();
        }

        void extraTitlebarElements(float textY) override {
            ImGui::SetCursorPosY(textY);

            ImGui::Text("%s | %s | Packet Buffer Size: %llu | Frame Time: %f | Polling Rate: ", targetName.c_str(),
                        interfName.c_str(), hwctrl::bytesWaiting(), ImGui::GetIO().DeltaTime);
            ImGui::SameLine(0, 5);
            ImGui::SetNextItemWidth(40_sc);
            ImGui::SetCursorPosY(textY);
            ImGui::InputInt("##pollrate", &hwctrl::POLLS_PER_UPDATE, 0);
            if(hwctrl::POLLS_PER_UPDATE < 1) hwctrl::POLLS_PER_UPDATE = 1;

            ImGui::SameLine();
            ImGui::SetCursorPosY(textY);

            hwctrl::update();
        }

        void menuPopupElements() override {
            if(ImGui::Button("Open Exports")) {
                openExportsFolder();
                ImGui::CloseCurrentPopup();
            }

            ImGui::Separator();

            if(ImGui::Button("Reset Window Layout")) {
                owner->preframe([this] { ImGui::LoadIniSettingsFromDisk(canonIni.c_str()); });
                ImGui::CloseCurrentPopup();
            }

            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, colors::BUTTON_CLOSE_HOVERED);
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, colors::BUTTON_CLOSE_ACTIVE);

            if(ImGui::TimedButton("CLOSE", closeTimer)) {
                ImGui::SameLine();
                ImGui::CircleProgressBar("##clearallprogressspinner", 7_sc, 2_sc, colors::TEXTi,
                                         closeTimer.timeSince() / WModule::CONFIRM_HOLD_TIME);

                if(closeTimer.timeSince() > WModule::CONFIRM_HOLD_TIME) owner->resetWindowlets();
            }

            ImGui::PopStyleColor(2);
        }

        bool lockCloseButton() override { return true; }
    };

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

    std::set<Windowlet*> makeWindowlets(TargetConfig& config) {
        const std::set<HardwareQualifier>& quals = hwctrl::getQuals();
        std::set<Windowlet*> wls;

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

            wls.insert(new Windowlet(title, std::move(wms)));
        }

        return wls;
    }

    class TargetChooserWM : public WModule {
        Window* const owner;

        std::vector<std::pair<std::string, InterfaceChooser*>> choosers;
        std::vector<std::pair<std::filesystem::path, std::string>> targetPaths;

        size_t chosenChooser;
        size_t chosenTarget;

        TargetConfig config;
        std::optional<std::string> jsonError;

    public:
        explicit TargetChooserWM(Window* owner) : owner(owner), chosenChooser(0), chosenTarget(0) {
            for(const auto& file : std::filesystem::directory_iterator("targets/")) {
                if(file.is_directory() || !file.path().string().ends_with(".json")) continue;
                std::filesystem::path path = file.path();
                targetPaths.emplace_back(path, path.filename().string());
            }

            choosers.emplace_back("Serial Port", new COMPortChooser());
            choosers.emplace_back("Virtual Port", new VirtualPortChooser());
            choosers.emplace_back("TCP Socket", new TCPInterfaceChooser());
        }

        ~TargetChooserWM() override {
            for(const auto& ch : choosers | std::views::values) delete ch;
        }

        void render() override {
            ImGui::PushID("TargetChooser");
            ImGui::PushID(classid);
            SCOPE_EXIT {
                ImGui::PopID();
                ImGui::PopID();
            };

            ImGui::Text("Choose Target Config: ");
            ImGui::SameLine();
            if(targetPaths.empty()) ImGui::Text("No Target Configs Available");
            else if(ImGui::BeginCombo("##targetchooser", targetPaths[chosenTarget].second.c_str())) {
                for(size_t i = 0; i < targetPaths.size(); i++) {
                    bool selected = i == chosenTarget;
                    if(ImGui::Selectable(targetPaths[i].second.c_str(), &selected)) chosenTarget = i;
                    if(selected) ImGui::SetItemDefaultFocus();
                }

                ImGui::EndCombo();
            }

            // Afterward a dropdown with the available interfaces is shown, and if a different interface chooser is
            // selected it is created
            ImGui::Text("Interface Type: ");
            ImGui::SameLine();

            const bool availableInterfaces = !choosers.empty();
            const bool lockDropdown = availableInterfaces && choosers[chosenChooser].second->shouldLockDropdown();

            if(lockDropdown) ImGui::BeginDisabled();

            if(!availableInterfaces) ImGui::Text("No available interfaces");
            else if(ImGui::BeginCombo("##interfacechooser", choosers[chosenChooser].first.c_str())) {
                for(size_t i = 0; i < choosers.size(); i++) {
                    bool selected = i == chosenChooser;
                    if(ImGui::Selectable((choosers[i].first + "##interfacechooser").c_str(), &selected)) {
                        chosenChooser = i;
                    }

                    if(selected) ImGui::SetItemDefaultFocus();
                }

                ImGui::EndCombo();
            }

            if(lockDropdown) ImGui::EndDisabled();

            ImGui::NewLine();
            ImGui::Separator();

            if(availableInterfaces) {
                choosers[chosenChooser].second->render();

                if(lockDropdown) ImGui::BeginDisabled();
                if(ImGui::Button("Connect")) {
                    auto& path = targetPaths[chosenTarget].first;

                    try {
                        config = readConfig(path);
                        choosers[chosenChooser].second->startConnection();
                    }

                    catch(nlohmann::json::exception& e) {
                        jsonError = e.what();
                    }
                }
                if(lockDropdown) ImGui::EndDisabled();

                if(jsonError.has_value()) {
                    ImGui::PushStyleColor(ImGuiCol_Text, colors::CERROR);
                    ImGui::TextWrapped("Error parsing config: %s", jsonError->c_str());
                    ImGui::PopStyleColor();

                    if(ImGui::Button("OK")) jsonError = std::nullopt;
                }

                RCP_Interface* interf = choosers[chosenChooser].second->getInterfAndCleanup();
                if(interf != nullptr) {
                    auto cpath = targetPaths[chosenTarget].first;
                    std::string userIniFile =
                        (getRoamingFolder() / "targets" / cpath.filename().string()).string() + ".ini";
                    std::string canonIniFile = cpath.string() + ".ini";
                    hwctrl::start(interf, config, cpath);
                    owner->setWindowlets(new TargetWTF(config.name, interf->interfaceType(), userIniFile, canonIniFile, owner),
                                         makeWindowlets(config));
                    owner->preframe([userIniFile] { ImGui::LoadIniSettingsFromDisk(userIniFile.c_str()); });
                }
            }
        }
    };
} // namespace

namespace LRI::RCI {
    TargetChooser::TargetChooser(Window* owner) : Windowlet("Target Chooser", {new TargetChooserWM(owner)}) {}

    void TargetChooser::render() {
        ImGui::SetNextWindowSize({550_sc, 225_sc}, ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowPos({50_sc, 90_sc}, ImGuiCond_FirstUseEver);

        ImGui::SetNextWindowViewport(ImGui::GetMainViewport()->ID);
        Windowlet::render();
    }
} // namespace LRI::RCI
