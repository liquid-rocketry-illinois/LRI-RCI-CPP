#include "UI/TargetChooser.h"

#include <Windows.h>
#include <SetupAPI.h>
#include <devguid.h>
#include <fstream>
#include <improgress.h>
#include <set>
#include <interfaces/VirtualPort.h>
#include <UI/CustomData.h>
#include <UI/EStop.h>
#include <UI/SensorReadings.h>
#include <UI/Solenoids.h>
#include <UI/Steppers.h>
#include <UI/TestControl.h>

#include "imgui.h"
#include "RCP_Host/RCP_Host.h"

#include "utils.h"
#include "RCP_Host_Impl.h"
#include "interfaces/COMPort.h"

namespace LRI::RCI {
    TargetChooser* TargetChooser::instance = nullptr;

    // This window always exists and is used to control the rest of the program
    TargetChooser::TargetChooser() : BaseUI(), interf(nullptr), chooser(nullptr), pollingRate(25), targetpaths(),
                                     targetconfig(), chosenConfig(0), interfaceoptions(), chosenInterface(0) {
        BaseUI::showWindow();

        // Iterate through the targets/ directory if it exists and create a list of the available targets
        if(std::filesystem::exists("targets/")) {
            for(const auto& file : std::filesystem::directory_iterator("targets/")) {
                if(file.is_directory() || !file.path().string().ends_with(".json")) continue;
                targetpaths.push_back(file.path().string());
            }
        }

        // Set up the two interface options
        interfaceoptions.emplace_back("Serial Port");
        interfaceoptions.emplace_back("Virtual Port");

        // Create the default chooser
        chooser = new COMPortChooser(this);
    }

    TargetChooser* TargetChooser::getInstance() {
        if(instance == nullptr) instance = new TargetChooser();
        return instance;
    }

    void TargetChooser::render() {
        ImGui::SetNextWindowPos(scale(ImVec2(50, 50)), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(scale(ImVec2(550, 225)), ImGuiCond_FirstUseEver);
        if(ImGui::Begin("Target Settings", nullptr, ImGuiWindowFlags_NoResize)) {
            ImGui::Text("Target Connection Status: ");
            ImGui::SameLine();

            // There are two different "modes" this window is in: when the interface is open and when it is not.
            if(interf && interf->isOpen()) {
                // When the interface is open, it shows the current target and interface configurations, as well as an
                // option to change the polling rate of RCP, in addition to a close button.
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0, 1, 0, 1));
                ImGui::Text("Open");
                ImGui::PopStyleColor();

                ImGui::Text("Current Target Config: %s", targetconfig["name"].get<std::string>().c_str());
                ImGui::Text("Current Interface: %s", interf->interfaceType().c_str());

                ImGui::NewLine();
                ImGui::Text("Polling Rate (polls/frame): ");
                ImGui::SameLine();
                ImGui::SetNextItemWidth(75 * scaling_factor);
                ImGui::InputInt("##pollinginput", &pollingRate, 1, 5);

                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1, 0, 0, 1));
                ImGui::Text("WARNING: Higher Polling Rates increase frame render time!");
                ImGui::PopStyleColor();
                ImGui::Text("Latest Frame Time (s): %f", ImGui::GetIO().DeltaTime);

                for(int i = 0; i < pollingRate && interf->pktAvailable(); i++) {
                    RCP_poll();
                }

                if(ImGui::Button("Close Interface")) {
                    RCP_shutdown();
                    delete interf;
                    interf = nullptr;
                    hideAll();
                }
            }

                // If an interface is not open, then a chooser should be rendered
            else {
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1, 0, 0, 1));
                ImGui::Text("Closed");
                ImGui::PopStyleColor();

                // First the user needs to select the target though
                ImGui::Text("Choose Target Config: ");
                ImGui::SameLine();
                if(targetpaths.empty()) ImGui::Text("No Target configs available");
                else if(ImGui::BeginCombo("##targetchoosecombo", targetpaths[chosenConfig].c_str())) {
                    for(size_t i = 0; i < targetpaths.size(); i++) {
                        bool selected = i == chosenConfig;
                        if(ImGui::Selectable((targetpaths[i] + "##targetchooser").c_str(), &selected))
                            chosenConfig = i;
                        if(selected) ImGui::SetItemDefaultFocus();
                    }
                    ImGui::EndCombo();
                }

                // Afterward a dropdown with the available interfaces is shown, and if a different interface chooser is
                // selected it is created
                ImGui::Text("Interface Type: ");
                ImGui::SameLine();
                bool choosermod = !chooser;
                if(interfaceoptions.empty()) ImGui::Text("No available interfaces");
                else if(ImGui::BeginCombo("##interfacechooser", interfaceoptions[chosenInterface].c_str())) {
                    for(size_t i = 0; i < interfaceoptions.size(); i++) {
                        bool selected = i == chosenInterface;
                        if(ImGui::Selectable((interfaceoptions[i] + "##interfacechooser").c_str(), &selected)) {
                            chosenInterface = i;
                            choosermod = true;
                        }

                        if(selected) ImGui::SetItemDefaultFocus();
                    }

                    ImGui::EndCombo();
                }

                if(choosermod) {
                    delete chooser;
                    switch(chosenInterface) {
                        case 0:
                            chooser = new COMPortChooser(this);
                            break;

                        case 1:
                            chooser = new VirtualPortChooser();
                            break;

                        default:
                            chooser = nullptr;
                    }
                }

                ImGui::NewLine();
                ImGui::Separator();

                // Render the chooser, and if it returns an open interface initialize RCP and the rest of the program
                if(chooser != nullptr) {
                    RCP_Interface* _interf = chooser->render();
                    if(_interf != nullptr) {
                        delete chooser;
                        chooser = nullptr;
                        interf = _interf;
                        RCP_init(callbacks);
                        RCP_setChannel(RCP_CH_ZERO);
                        std::ifstream config(targetpaths[chosenConfig]);
                        targetconfig = nlohmann::json::parse(config);
                        initWindows();
                    }
                }
            }
        }

        ImGui::End();
    }

    const RCP_Interface* TargetChooser::getInterface() const {
        return interf;
    }

    // Hide and show are blank because this window should always be present
    void TargetChooser::hideWindow() {}

    void TargetChooser::showWindow() {}

    // Helper function that resets and initializes all windows
    void TargetChooser::initWindows() {
        CustomData::getInstance()->reset();
        SensorReadings::getInstance()->reset();
        Solenoids::getInstance()->reset();
        Steppers::getInstance()->reset();
        TestControl::getInstance()->reset();

        // These 3 can be shown regardless of the target
        TestControl::getInstance()->showWindow();
        EStop::getInstance()->showWindow();
        CustomData::getInstance()->showWindow();

        // Iterate through all the devices in the json and initialize the appropriate windows
        std::set<SensorQualifier> sensors;
        for(int i = 0; i < targetconfig["devices"].size(); i++) {
            switch(targetconfig["devices"][i]["devclass"].get<int>()) {
                case RCP_DEVCLASS_SOLENOID: {
                    auto ids = targetconfig["devices"][i]["ids"].get<std::vector<uint8_t>>();
                    auto names = targetconfig["devices"][i]["names"].get<std::vector<std::string>>();
                    std::map<uint8_t, std::string> sols;
                    if(ids.size() != names.size()) break;

                    for(size_t j = 0; j < ids.size(); j++) sols[ids[j]] = names[j];
                    Solenoids::getInstance()->setHardwareConfig(sols);
                    Solenoids::getInstance()->showWindow();
                    break;
                }

                case RCP_DEVCLASS_STEPPER: {
                    auto ids = targetconfig["devices"][i]["ids"].get<std::vector<uint8_t>>();
                    auto names = targetconfig["devices"][i]["names"].get<std::vector<std::string>>();
                    if(ids.size() != names.size()) break;

                    std::map<uint8_t, std::string> steps;
                    for(size_t j = 0; j < ids.size(); j++) steps[ids[j]] = names[j];
                    Steppers::getInstance()->setHardwareConfig(steps);
                    Steppers::getInstance()->showWindow();
                    break;
                }

                case RCP_DEVCLASS_PRESSURE_TRANSDUCER: {
                    auto ids = targetconfig["devices"][i]["ids"].get<std::vector<uint8_t>>();
                    auto names = targetconfig["devices"][i]["names"].get<std::vector<std::string>>();
                    if(ids.size() != names.size()) break;

                    for(size_t j = 0; j < ids.size(); j++)
                        sensors.insert({
                                               .devclass = RCP_DEVCLASS_PRESSURE_TRANSDUCER,
                                               .id = ids[j],
                                               .name = names[j]
                                       });

                    break;
                }

                case RCP_DEVCLASS_GPS:
                    sensors.insert({.devclass = RCP_DEVCLASS_GPS, .name = "GPS"});
                    break;

                case RCP_DEVCLASS_MAGNETOMETER:
                    sensors.insert({.devclass = RCP_DEVCLASS_MAGNETOMETER, .name = "Magnetometer"});
                    break;

                case RCP_DEVCLASS_AM_PRESSURE:
                    sensors.insert({.devclass = RCP_DEVCLASS_AM_PRESSURE, .name = "Ambient Pressure"});
                    break;

                case RCP_DEVCLASS_AM_TEMPERATURE:
                    sensors.insert({.devclass = RCP_DEVCLASS_AM_TEMPERATURE, .name = "Ambient Temperature"});
                    break;

                case RCP_DEVCLASS_ACCELEROMETER:
                    sensors.insert({.devclass = RCP_DEVCLASS_ACCELEROMETER, .name = "Accelerometer"});
                    break;

                case RCP_DEVCLASS_GYROSCOPE:
                    sensors.insert({.devclass = RCP_DEVCLASS_GYROSCOPE, .name = "Gyroscope"});
                    break;

                default:
                    break;
            }
        }

        // If there is an actual sensor present, then display the sensors window
        if(!sensors.empty()) {
            SensorReadings::getInstance()->setHardwareConfig(sensors);
            SensorReadings::getInstance()->showWindow();
        }
    }

    TargetChooser::InterfaceChooser::InterfaceChooser(TargetChooser* _targetchooser) : targetchooser(_targetchooser) {}

    // The COMPort chooser will enumerate all available serial devices to be picked from
    TargetChooser::COMPortChooser::COMPortChooser(TargetChooser* targetchooser) : InterfaceChooser(targetchooser),
                                                                                  portlist(), selectedPort(0),
                                                                                  error(false), baud(115200),
                                                                                  port(nullptr) {
        enumSerialDevs();
    }

    // Honestly I dont know what this does its some Windows spaghetti I stole from SO but it works so yay
    // https://stackoverflow.com/a/77752863
    bool TargetChooser::COMPortChooser::enumSerialDevs() {
        portlist.clear();
        HANDLE devs = SetupDiGetClassDevs(&GUID_DEVCLASS_PORTS, nullptr, nullptr, DIGCF_PRESENT);
        if(devs == INVALID_HANDLE_VALUE) return false;

        SP_DEVINFO_DATA data;
        data.cbSize = sizeof(SP_DEVINFO_DATA);
        char s[80];

        for(DWORD i = 0; SetupDiEnumDeviceInfo(devs, i, &data); i++) {
            HKEY hkey = SetupDiOpenDevRegKey(devs, &data, DICS_FLAG_GLOBAL, 0, DIREG_DEV, KEY_READ);
            if(hkey == INVALID_HANDLE_VALUE) {
                return false;
            }

            char comname[16] = {0};
            DWORD len = 16;

            RegQueryValueEx(hkey, "PortName", nullptr, nullptr, (LPBYTE) comname, &len);
            RegCloseKey(hkey);
            if(comname[0] != 'C') continue;

            SetupDiGetDeviceRegistryProperty(devs, &data, SPDRP_FRIENDLYNAME, nullptr, (PBYTE) s, sizeof(s), nullptr);

            // Somehow we end up with the name we need to open the port, and a more user friendly display string.
            // These get appended to this vector for later
            portlist.push_back(std::string(comname) + ": " + std::string(s));
        }

        SetupDiDestroyDeviceInfoList(devs);
        return true;
    }

    // The COMPort chooser rendering function
    RCP_Interface* TargetChooser::COMPortChooser::render() {
        bool disable = port;
        if(disable) ImGui::BeginDisabled();

        // It has a dropdown for which device, as listed in enumSerial()
        ImGui::Text("Choose Serial Port: ");
        ImGui::SameLine();
        if(portlist.empty()) ImGui::Text("No Ports Detected");
        else if(ImGui::BeginCombo("##portselectcombo", portlist[selectedPort].c_str())) {
            for(size_t i = 0; i < portlist.size(); i++) {
                bool selected = i == selectedPort;
                if(ImGui::Selectable((portlist[i] + "##portselectcombo").c_str(), &selected))
                    selectedPort = i;
                if(selected) ImGui::SetItemDefaultFocus();
            }

            ImGui::EndCombo();
        }

        if(ImGui::Button("Refresh List")) {
            selectedPort = 0;
            enumSerialDevs();
        }

        // Input for baud rate
        ImGui::Text("Baud Rate: ");
        ImGui::SameLine();
        ImGui::SetNextItemWidth(scale(100));
        ImGui::InputInt("##comportchooserbaudinput", &baud);
        if(baud < 0) baud = 0;
        else if(baud > 500'000) baud = 500'000;

        if(portlist.empty()) ImGui::BeginDisabled();
        if(ImGui::Button("Connect")) {
            // If connect, then create the COMPort
            port = new COMPort(
                    portlist[selectedPort].substr(0, portlist[selectedPort].find_first_of(':')).c_str(), baud);
        }
        if(portlist.empty()) ImGui::EndDisabled();
        if(disable) ImGui::EndDisabled();

        // If the port failed to allocate then return
        if(!port) return nullptr;

        // If the port allocated but did not open then show an error
        if(!port->isOpen()) {
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1, 0, 0, 1));
            ImGui::Text("Error Connecting to Serial Port (%u)", port->lastError());
            ImGui::PopStyleColor();

            ImGui::SameLine();
            if(ImGui::Button("OK##comportchoosererror")) {
                delete port;
                port = nullptr;
            }
        }

        // While the port is readying, don't return it just yet and display a loading spinner
        else if(!port->isReady()) {
            ImGui::SameLine();
            ImGui::Spinner("##comportchooserspinner", 8, 1, REBECCA_PURPLE);
        }

        else return port;
        return nullptr;
    }

    TargetChooser::VirtualPortChooser::VirtualPortChooser() : InterfaceChooser(nullptr) {
    }

    // Virtual port is easy
    RCP_Interface* TargetChooser::VirtualPortChooser::render() {
        if(ImGui::Button("Open Virtual Port")) return new VirtualPort();
        return nullptr;
    }
}
