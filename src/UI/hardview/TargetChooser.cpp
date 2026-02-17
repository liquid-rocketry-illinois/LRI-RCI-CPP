#include "UI/TargetChooser.h"

#include <fstream>
#include <set>
#include <shellapi.h>

#include "RCP_Host/RCP_Host.h"
#include "imgui.h"
#include "improgress.h"

#include "utils.h"

#include "interfaces/COMPort.h"
#include "interfaces/TCPSocket.h"
#include "interfaces/VirtualPort.h"

#include "hardware/AngledActuator.h"
#include "hardware/BoolSensor.h"
#include "hardware/HardwareControl.h"
#include "hardware/Motors.h"
#include "hardware/Prompt.h"
#include "hardware/Sensors.h"
#include "hardware/SimpleActuators.h"
#include "hardware/Steppers.h"
#include "hardware/TargetLog.h"
#include "hardware/TestState.h"

#include "../../../include/UI/hardview/AngledActuatorViewer.h"
#include "../../../include/UI/hardview/MotorViewer.h"
#include "UI/BoolSensorViewer.h"
#include "UI/EStopViewer.h"
#include "UI/PromptViewer.h"
#include "UI/RawViewer.h"
#include "UI/SensorViewer.h"
#include "UI/SimpleActuatorViewer.h"
#include "UI/StepperViewer.h"
#include "UI/TestStateViewer.h"

// The important one. Module for controlling the program as a whole
namespace LRI::RCI {
    TargetChooser::TargetChooser(ControlWindowlet* control) :
        control(control), pollingRate(25), chooser(nullptr), chosenConfig(0), chosenInterface(0), activeTarget(false) {
        // Iterate through the targets/ directory if it exists and create a list of the available targets
        auto targetsFolder = getRoamingFolder() / "targets/";
        for(const auto& file : std::filesystem::directory_iterator(targetsFolder)) {
            if(file.is_directory() || !file.path().string().ends_with(".json")) continue;
            targetpaths.push_back(file);
        }

        // Set up the two interface options
        interfaceoptions.emplace_back("Serial Port");
        interfaceoptions.emplace_back("Virtual Port");
        interfaceoptions.emplace_back("TCP Socket");

        // Create the default chooser
        chooser = new COMPortChooser();
    }

    void TargetChooser::render() {
        ImGui::PushID("TargetChooser");

        ImGui::Text("Target Connection Status: ");
        ImGui::SameLine();

        // There are two different "modes" this window is in: when the interface is open and when it is not.
        if(activeTarget) {
            // When the interface is open, it shows the current target and interface configurations, as well as an
            // option to change the polling rate of RCP, in addition to a close button.
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0, 1, 0, 1));
            ImGui::Text("Open");
            ImGui::PopStyleColor();

            ImGui::Text("Current Target Config: %s", configName.c_str());
            ImGui::Text("Current Interface: %s", interfName.c_str());

            ImGui::NewLine();
            ImGui::Text("Polling Rate (polls/frame): ");
            ImGui::SameLine();
            ImGui::SetNextItemWidth(75 * scaling_factor);
            ImGui::InputInt("##pollinginput", &HWCTRL::POLLS_PER_UPDATE, 1, 5);
            if(HWCTRL::POLLS_PER_UPDATE < 1) HWCTRL::POLLS_PER_UPDATE = 1;

            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1, 0, 0, 1));
            ImGui::Text("WARNING: Higher Polling Rates increase frame render time!");
            ImGui::PopStyleColor();
            ImGui::Text("Latest Frame Time (s): %f", ImGui::GetIO().DeltaTime);

            if(ImGui::Button("Close Interface")) {
                HWCTRL::end();
                control->cleanup();
                activeTarget = false;
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
            else if(ImGui::BeginCombo("##targetchoosecombo", targetpaths[chosenConfig].filename().string().c_str())) {
                for(size_t i = 0; i < targetpaths.size(); i++) {
                    bool selected = i == chosenConfig;
                    if(ImGui::Selectable(targetpaths[i].filename().string().c_str(), &selected)) chosenConfig = i;
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

            // If a new chooser has been chosen:
            if(choosermod) {
                delete chooser;
                switch(chosenInterface) {
                case 0:
                    chooser = new COMPortChooser();
                    break;

                case 1:
                    chooser = new VirtualPortChooser();
                    break;

                case 2:
                    chooser = new TCPInterfaceChooser();
                    break;

                default:
                    chooser = nullptr;
                }
            }

            ImGui::NewLine();
            ImGui::Separator();

            // Render the chooser, and if it returns an open interface initialize RCP and the rest of the program
            if(chooser != nullptr) {
                RCP_Interface* interf = chooser->render();
                if(interf != nullptr) {
                    // If the chooser indicates success:
                    delete chooser;
                    chooser = nullptr;
                    interfName = interf->interfaceType();
                    HWCTRL::start(interf);

                    // Parse the selected config file
                    std::ifstream config(targetpaths[chosenConfig]);
                    targetconfig = nlohmann::json::parse(config);

                    std::filesystem::path inipath =
                        getRoamingFolder() / "targets" / (targetpaths[chosenConfig].filename().string() + ".ini");
                    control->inipath = inipath.string();

                    // Tell the main loop to load the new ini file before the next frame
                    if(std::filesystem::exists(control->inipath)) iniFilePath.path = control->inipath;

                    // Call initializer of the rest of the windows
                    initWindows();
                    activeTarget = true;
                }
            }
        }

        ImGui::NewLine();
        if(ImGui::Button("Open Exports")) {
            ShellExecute(nullptr, "open", (getRoamingFolder() / "exports").string().c_str(), nullptr, nullptr,
                         SW_SHOWDEFAULT);
        }

        if(activeTarget) {
            ImGui::SameLine();
            if(ImGui::Button("Reset Layout")) {
                std::filesystem::path inifile = control->inipath;
                std::filesystem::path origfile = "targets" / inifile.filename();
                if(std::filesystem::exists(origfile) && !std::filesystem::is_directory(origfile)) {
                    std::filesystem::remove(inifile);
                    std::filesystem::copy(origfile, inifile);
                    iniFilePath.path = inifile.string();
                }
            }
        }

        ImGui::PopID();
    }

    std::set<HardwareQualifier> TargetChooser::getValidQuals(const std::set<HardwareQualifier>& allquals,
                                                             RCP_DeviceClass devclass, const std::set<uint8_t>& ids) {
        std::set<uint8_t> validIds;

        for(const auto& id : ids) {
            HardwareQualifier temp = {devclass, id};
            if(!allquals.contains(temp)) {
                HWCTRL::addError({HWCTRL::ErrorType::HWNE_HOST, temp});
                continue;
            }

            validIds.insert(id);
        }

        auto filtered = allquals | std::views::filter([&validIds, devclass](const HardwareQualifier& qual) {
                            return qual.devclass == devclass && validIds.contains(qual.id);
                        });

        return std::set<HardwareQualifier>{filtered.cbegin(), filtered.cend()};
    }


    // Helper function that resets and initializes everything based on the configuration file
    void TargetChooser::initWindows() {
        configName = targetconfig["name"].get<std::string>();

        // Parse test list and pass to TestState
        std::map<uint8_t, std::string> tests;
        for(size_t i = 0; i < targetconfig["tests"].size(); i++) {
            tests[targetconfig["tests"][i]["id"].get<uint8_t>()] = targetconfig["tests"][i]["name"].get<std::string>();
        }
        TestState::setTests(tests);

        // Create a master set of all qualifiers
        std::set<HardwareQualifier> allquals;

        // Create a set of all sensors, besides the bool sensor
        std::set<HardwareQualifier> sensors;

        // Reset the debug log and prompts
        RawData::reset();
        Prompt::reset();

        // Iterate through all devices
        for(size_t i = 0; i < targetconfig["devices"].size(); i++) {
            // Get the device class of this object
            auto devclass = static_cast<RCP_DeviceClass>(targetconfig["devices"][i]["devclass"].get<int>());

            // Get the name and id arrays
            auto ids = targetconfig["devices"][i]["ids"].get<std::vector<uint8_t>>();
            auto names = targetconfig["devices"][i]["names"].get<std::vector<std::string>>();

            // If the lengths dont match then drop this section
            if(ids.empty() || ids.size() != names.size()) continue;

            // Construct the qualifiers in this section
            std::set<HardwareQualifier> quals;
            for(size_t j = 0; j < ids.size(); j++) quals.insert(HardwareQualifier{devclass, ids[j], names[j]});

            // Insert the constructed qualifiers into the master qualifier list
            allquals.insert(quals.begin(), quals.end());

            // Switch on the device class and configure the appropriate singleton
            switch(devclass) {
            case RCP_DEVCLASS_STEPPER:
                Steppers::setHardwareConfig(quals);
                break;

            case RCP_DEVCLASS_MOTOR:
                Motors::setHarwareConfig(quals);
                break;

            case RCP_DEVCLASS_SIMPLE_ACTUATOR:
                SimpleActuators::setHardwareConfig(quals);
                break;

            case RCP_DEVCLASS_BOOL_SENSOR:
                BoolSensors::setHardwareConfig(quals, targetconfig["devices"][i]["refreshTime"].get<int>());
                break;

            case RCP_DEVCLASS_ANGLED_ACTUATOR:
                // Intentional case fallthrough
                AngledActuators::setHardwareConfig(quals);
                [[fallthrough]];

            case RCP_DEVCLASS_AM_PRESSURE:
            case RCP_DEVCLASS_TEMPERATURE:
            case RCP_DEVCLASS_PRESSURE_TRANSDUCER:
            case RCP_DEVCLASS_RELATIVE_HYGROMETER:
            case RCP_DEVCLASS_LOAD_CELL:
            case RCP_DEVCLASS_POWERMON:
            case RCP_DEVCLASS_ACCELEROMETER:
            case RCP_DEVCLASS_GYROSCOPE:
            case RCP_DEVCLASS_MAGNETOMETER:
            case RCP_DEVCLASS_GPS:
                sensors.insert(quals.cbegin(), quals.cend());
                break;

            default:
                break;
            }
        }

        // Load the sensors singleton with all the sensor qualifiers
        Sensors::setHardwareConfig(sensors);

        // Iterate over the windowlets and configure their modules
        for(size_t i = 0; i < targetconfig["windows"].size(); i++) {
            std::vector<WModule*> modules;

            // Iterate over the modules array
            for(size_t j = 0; j < targetconfig["windows"][i]["modules"].size(); j++) {
                // get the module type
                int type = targetconfig["windows"][i]["modules"][j]["type"].get<int>();

                // Switch on the type and parse the correct parameters
                switch(type) {
                case -1:
                    modules.push_back(new EStopViewer());
                    break;

                case RCP_DEVCLASS_TEST_STATE:
                    modules.push_back(new TestStateViewer());
                    break;

                case RCP_DEVCLASS_MOTOR:
                case RCP_DEVCLASS_SIMPLE_ACTUATOR:
                case RCP_DEVCLASS_BOOL_SENSOR:
                case RCP_DEVCLASS_STEPPER:
                case RCP_DEVCLASS_ANGLED_ACTUATOR: {
                    bool refresh = targetconfig["windows"][i]["modules"][j]["refresh"].get<bool>();
                    auto ids = targetconfig["windows"][i]["modules"][j]["ids"].get<std::set<uint8_t>>();

                    const auto qualSet = getValidQuals(allquals, static_cast<RCP_DeviceClass>(type), ids);

                    // Determine correct module type and construct it
                    if(type == RCP_DEVCLASS_SIMPLE_ACTUATOR)
                        modules.push_back(new SimpleActuatorViewer(qualSet, refresh));
                    else if(type == RCP_DEVCLASS_STEPPER) modules.push_back(new StepperViewer(qualSet, refresh));
                    else if(type == RCP_DEVCLASS_ANGLED_ACTUATOR)
                        modules.push_back(new AngledActuatorViewer(qualSet, refresh));
                    else if(type == RCP_DEVCLASS_MOTOR) modules.push_back(new MotorViewer(qualSet, refresh));
                    else modules.push_back(new BoolSensorViewer(qualSet, refresh));
                    break;
                }

                case RCP_DEVCLASS_PROMPT:
                    modules.push_back(new PromptViewer());
                    break;

                case RCP_DEVCLASS_CUSTOM:
                    modules.push_back(new RawViewer());
                    break;

                case RCP_DEVCLASS_AM_PRESSURE:
                case RCP_DEVCLASS_TEMPERATURE:
                case RCP_DEVCLASS_PRESSURE_TRANSDUCER:
                case RCP_DEVCLASS_RELATIVE_HYGROMETER:
                case RCP_DEVCLASS_LOAD_CELL:
                case RCP_DEVCLASS_POWERMON:
                case RCP_DEVCLASS_ACCELEROMETER:
                case RCP_DEVCLASS_GYROSCOPE:
                case RCP_DEVCLASS_MAGNETOMETER:
                case RCP_DEVCLASS_GPS: {
                    // Get abridged setting
                    bool abridged = targetconfig["windows"][i]["modules"][j]["abridged"].get<bool>();
                    std::set<HardwareQualifier> quals;

                    // Parse which qualifiers to add
                    for(size_t k = 0; k < targetconfig["windows"][i]["modules"][j]["ids"].size(); k++) {
                        // Json's getting a little long lol
                        int devclass = targetconfig["windows"][i]["modules"][j]["ids"][k]["class"].get<int>();
                        auto ids = targetconfig["windows"][i]["modules"][j]["ids"][k]["ids"].get<std::set<uint8_t>>();

                        const auto qualSet = getValidQuals(allquals, static_cast<RCP_DeviceClass>(devclass), ids);
                        quals.insert(qualSet.cbegin(), qualSet.cend());
                    }

                    modules.push_back(new SensorViewer(quals, abridged));
                    break;
                }

                default:
                    break;
                }
            }

            // The pointer to windowlet does not need to be saved, since the windowlet constructor automatically
            // adds itself to the global set
            new Windowlet(targetconfig["windows"][i]["title"].get<std::string>(), modules);
        }
    }

    int InterfaceChooser::CLASSID = 0;

    InterfaceChooser::InterfaceChooser() : classid(CLASSID++) {}

    COMPortChooser::COMPortChooser() : selectedPort(0), error(false), baud(115200), port(nullptr) {
        COMPort::enumSerialDevs(portlist);
    }

    RCP_Interface* COMPortChooser::render() {
        ImGui::PushID("COMPortChooser");
        ImGui::PushID(classid);

        bool disable = port;
        if(disable) ImGui::BeginDisabled();

        // It has a dropdown for which device, as listed in enumSerial()
        ImGui::Text("Choose Serial Port: ");
        ImGui::SameLine();
        if(portlist.empty()) ImGui::Text("No Ports Detected");
        else if(ImGui::BeginCombo("##portselectcombo", portlist[selectedPort].second.c_str())) {
            for(size_t i = 0; i < portlist.size(); i++) {
                ImGui::PushID(static_cast<int>(i));
                bool selected = i == selectedPort;
                if(ImGui::Selectable((portlist[i]).second.c_str(), &selected)) selectedPort = i;
                if(selected) ImGui::SetItemDefaultFocus();
                ImGui::PopID();
            }

            ImGui::EndCombo();
        }

        if(ImGui::Button("Refresh List")) {
            selectedPort = 0;
            COMPort::enumSerialDevs(portlist);
        }

        // Input for baud rate
        ImGui::Text("Baud Rate: ");
        ImGui::SameLine();
        ImGui::SetNextItemWidth(scale(100));
        ImGui::InputInt("##comportchooserbaudinput", &baud);
        if(baud < 0) baud = 0;
        else if(baud > 500'000) baud = 500'000;

        ImGui::SameLine();
        ImGui::Text(" | Arduino Mode: ");
        ImGui::SameLine();
        ImGui::Checkbox("##arduinomode", &arduinoMode);

        if(portlist.empty()) ImGui::BeginDisabled();
        if(ImGui::Button("Connect")) {
            // If connect, then create the COMPort
            port = new COMPort(std::move(R"(\\.\)" + portlist[selectedPort].first), baud);
        }
        if(portlist.empty()) ImGui::EndDisabled();
        if(disable) ImGui::EndDisabled();

        // If the port failed to allocate then return
        if(!port) {
            ImGui::PopID();
            ImGui::PopID();
            return nullptr;
        }

        // If the port allocated but did not open then show an error
        if(port->didPortOpenFail()) {
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1, 0, 0, 1));
            ImGui::Text("Error Connecting to Serial Port stage %lu code %lu)", port->lastError().code,
                        port->lastError().stage);
            ImGui::PopStyleColor();

            ImGui::SameLine();
            if(ImGui::Button("OK##comportchoosererror")) {
                delete port;
                port = nullptr;
            }
        }

        // While the port is readying, don't return it just yet and display a loading spinner
        else if(!port->isOpen()) {
            ImGui::SameLine();
            ImGui::Spinner("##comportchooserspinner", 8, 1, WModule::REBECCA_PURPLE);
        }

        else {
            ImGui::PopID();
            ImGui::PopID();
            return port;
        }

        ImGui::PopID();
        ImGui::PopID();
        return nullptr;
    }

    // Chooser for the interface. Just needs port, client/server, and server ip address if client
    RCP_Interface* TCPInterfaceChooser::render() {
        ImGui::PushID("TCPSocketChooser");
        ImGui::PushID(classid);

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
            ImGui::SetNextItemWidth(scale(200));
            ImGui::InputInt4("##serveraddrinput", ip);
            for(int& i : ip) {
                if(i < 0) i = 0;
                else if(i > 255) i = 255;
            }
        }

        // Port input
        ImGui::Text("Port: ");
        ImGui::SameLine();
        ImGui::SetNextItemWidth(scale(48));
        ImGui::InputInt("##portinput", &port, 0);
        if(port < 0) port = 0;
        else if(port > 65535) port = 65535;

        // Once confirm is pushed, create the interface and begin waiting for a connection
        if(ImGui::Button(tempserver ? "Begin Hosting" : "Connect")) {
            interf =
                new TCPSocket(port, tempserver ? sf::IpAddress(0, 0, 0, 0) : sf::IpAddress(ip[0], ip[1], ip[2], ip[3]));
        }
        if(!isnull) ImGui::EndDisabled();

        // If the interface is null, keep rendering
        if(interf == nullptr) {
            ImGui::PopID();
            ImGui::PopID();
            return nullptr;
        }

        // If the interface has been created but did not open properly, display error and continue rendering
        if(interf->didPortOpenFail()) {
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1, 0, 0, 1));
            TCPSocket::Error error = interf->lastError();
            ImGui::Text("Error opening TCP socket: stage %lu, code %lu", error.stage, error.code);
            ImGui::PopStyleColor();

            ImGui::SameLine();
            if(ImGui::Button("OK")) {
                delete interf;
                ImGui::PopID();
                ImGui::PopID();
                interf = nullptr;
            }

            ImGui::PopID();
            ImGui::PopID();
            return nullptr;
        }

        // While the interface is open but not ready, keep waiting for the connection to be established
        if(!interf->isOpen()) {
            ImGui::SameLine();
            ImGui::Text("Waiting for connection");
            ImGui::SameLine();
            ImGui::Spinner("##tcpwaitspinner", 8, 1, WModule::REBECCA_PURPLE);

            if(ImGui::Button("Cancel")) {
                delete interf;
                ImGui::PopID();
                ImGui::PopID();
                interf = nullptr;
            }

            ImGui::PopID();
            ImGui::PopID();
            return nullptr;
        }

        ImGui::PopID();
        ImGui::PopID();
        // Once the interface is ready to go, return it to the TargetChooser
        return interf;
    }

    // Virtual port is easy
    RCP_Interface* VirtualPortChooser::render() {
        if(ImGui::Button("Open Virtual Port")) return new VirtualPort();
        return nullptr;
    }
} // namespace LRI::RCI
