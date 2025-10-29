#include "UI/TargetChooser.h"

#include <fstream>
#include <set>

#include "RCP_Host/RCP_Host.h"
#include "imgui.h"

#include "utils.h"

#include "interfaces/COMPort.h"
#include "interfaces/TCPSocket.h"
#include "interfaces/VirtualPort.h"

#include "hardware/AngledActuator.h"
#include "hardware/BoolSensor.h"
#include "hardware/HardwareControl.h"
#include "hardware/Prompt.h"
#include "hardware/RawData.h"
#include "hardware/Sensors.h"
#include "hardware/SimpleActuators.h"
#include "hardware/Steppers.h"
#include "hardware/TestState.h"

#include "UI/AngledActuatorViewer.h"
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
        if(std::filesystem::exists("targets/")) {
            for(const auto& file : std::filesystem::directory_iterator("targets/")) {
                if(file.is_directory() || !file.path().string().ends_with(".json")) continue;
                targetpaths.push_back(file.path().string());
            }
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
            else if(ImGui::BeginCombo("##targetchoosecombo", targetpaths[chosenConfig].c_str())) {
                for(size_t i = 0; i < targetpaths.size(); i++) {
                    bool selected = i == chosenConfig;
                    if(ImGui::Selectable((targetpaths[i] + "##targetchooser").c_str(), &selected)) chosenConfig = i;
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
                    control->inipath = targetpaths[chosenConfig] + ".ini";

                    // Tell the main loop to load the new ini file before the next frame
                    if(std::filesystem::exists(control->inipath)) iniFilePath.path = control->inipath;

                    // Call initializer of the rest of the windows
                    initWindows();
                    activeTarget = true;
                }
            }
        }

        ImGui::PopID();
    }

    // Helper function that resets and initializes everything based on the configuration file
    void TargetChooser::initWindows() {
        configName = targetconfig["name"].get<std::string>();

        // Parse test list and pass to TestState
        std::map<uint8_t, std::string> tests;
        for(size_t i = 0; i < targetconfig["tests"].size(); i++) {
            tests[targetconfig["tests"][i]["id"].get<uint8_t>()] = targetconfig["tests"][i]["name"].get<std::string>();
        }
        TestState::getInstance()->setTests(tests);

        // Create a master set of all qualifiers
        std::set<HardwareQualifier> allquals;

        // Create a set of all sensors, besides the bool sensor
        std::set<HardwareQualifier> sensors;

        // Reset the debug log and prompts
        RawData::getInstance()->reset();
        Prompt::getInstance()->reset();

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
                Steppers::getInstance()->setHardwareConfig(quals);
                break;

            case RCP_DEVCLASS_SIMPLE_ACTUATOR:
                SimpleActuators::getInstance()->setHardwareConfig(quals);
                break;

            case RCP_DEVCLASS_BOOL_SENSOR:
                BoolSensors::getInstance()->setHardwareConfig(quals,
                                                              targetconfig["devices"][i]["refreshTime"].get<int>());
                break;

            case RCP_DEVCLASS_ANGLED_ACTUATOR:
                // Intentional case fallthrough
                AngledActuators::setHardwareConfig(quals);
                [[fallthrough]];

            case RCP_DEVCLASS_AM_PRESSURE:
            case RCP_DEVCLASS_AM_TEMPERATURE:
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
        Sensors::getInstance()->setHardwareConfig(sensors);

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

                case RCP_DEVCLASS_SIMPLE_ACTUATOR:
                case RCP_DEVCLASS_BOOL_SENSOR:
                case RCP_DEVCLASS_STEPPER:
                case RCP_DEVCLASS_ANGLED_ACTUATOR: {
                    bool refresh = targetconfig["windows"][i]["modules"][j]["refresh"].get<bool>();
                    auto ids = targetconfig["windows"][i]["modules"][j]["ids"].get<std::set<int>>();

                    // Filter out any qualifiers that havent been configured in the devices section
                    auto filtered = allquals | std::views::filter([&type, &ids](const HardwareQualifier& q) {
                                        return q.devclass == type && ids.contains(q.id);
                                    });

                    const auto qualSet = std::set(filtered.begin(), filtered.end());

                    // Determine correct module type and construct it
                    if(type == RCP_DEVCLASS_SIMPLE_ACTUATOR)
                        modules.push_back(new SimpleActuatorViewer(qualSet, refresh));
                    else if(type == RCP_DEVCLASS_STEPPER) modules.push_back(new StepperViewer(qualSet, refresh));
                    else if(type == RCP_DEVCLASS_ANGLED_ACTUATOR)
                        modules.push_back(new AngledActuatorViewer(qualSet, refresh));
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
                case RCP_DEVCLASS_AM_TEMPERATURE:
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
                        auto ids = targetconfig["windows"][i]["modules"][j]["ids"][k]["ids"].get<std::set<int>>();

                        // Filter out any qualifiers not configured in the devices section
                        auto filtered = allquals | std::views::filter([&devclass, &ids](const HardwareQualifier& q) {
                                            return q.devclass == devclass && ids.contains(q.id);
                                        });
                        quals.insert(filtered.begin(), filtered.end());
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
} // namespace LRI::RCI
