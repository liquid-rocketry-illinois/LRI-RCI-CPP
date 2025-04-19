#include "UI/TargetChooser.h"

#include <fstream>
#include <set>

#include "imgui.h"
#include "RCP_Host/RCP_Host.h"

#include "RCP_Host_Impl.h"
#include "utils.h"

#include "interfaces/COMPort.h"
#include "interfaces/TCPSocket.h"
#include "interfaces/VirtualPort.h"

#include "hardware/BoolSensor.h"
#include "hardware/Prompt.h"
#include "hardware/RawData.h"
#include "hardware/Sensors.h"
#include "hardware/SimpleActuators.h"
#include "hardware/Steppers.h"
#include "hardware/TestState.h"

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
        control(control), interf(nullptr), pollingRate(25), chooser(nullptr), chosenConfig(0), chosenInterface(0) {
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
        if(interf && interf->isOpen()) {
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
                control->cleanup();
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
                RCP_Interface* _interf = chooser->render();
                if(_interf != nullptr) {
                    // If the chooser indicates success:
                    delete chooser;
                    chooser = nullptr;
                    interf = _interf;
                    RCP_init(callbacks);
                    RCP_setChannel(RCP_CH_ZERO);

                    // Parse the selected config file
                    std::ifstream config(targetpaths[chosenConfig]);
                    targetconfig = nlohmann::json::parse(config);
                    control->interf = interf;
                    control->inipath = targetpaths[chosenConfig] + ".ini";

                    // Tell the main loop to load the new ini file before the next frame
                    if(std::filesystem::exists(targetpaths[chosenConfig] + ".ini")) iniFilePath.path = control->inipath;

                    // Call initializer of the rest of the windows
                    initWindows();
                }
            }
        }

        ImGui::PopID();
    }

    /*
             * This is the json parser. The json schema is a little weird so it is described here. Each
             * field is given, what data type it has, and so on:
             *  - name: string: The name of the configuraion. Used only for display purposes
             *  - tests: array of objects: Used to configure the available autosequenced tests and what their
             *    RCP id [0, 15] is. Each object contains the fields:
             *    - id: The numeric ID of the test
             *    - name: Human readable name of the test
             *  - devices: an array of objects. Used to configure the singletons with what devices
             *    are present and should be loaded. Two objects in this array cannot share the same
             *    device class. Each object contains the fields:
             *    - devclass: int: the RCP device class associated with the device. Classes 0 and 0x80
             *      do not need to be specified for them to function
             *    - ids: array of ints: indicates the IDs to load. Even if there is only one of a particular
             *      device class this still needs to be an array of ints
             *    - names: array of strings: matches human readably names to the IDs of the devices. Must be in
             *      the same order as the IDs array and must be the same size. At least one name is required to
             *      match the one required ID
             *  - windows: an array of objects. Used to configure the windowlets and which modules they contain.
             *    Each object contians the fields:
             *    - title: string: title of the windowlet
             *    - modules: an array of objects: gives an ordered list of what modules should appear in that
             *      windowlet. Each object has the fields:
             *      - type: int: indicates the type of module, typically following RCP device classes. Depending
             *        on the type, the remaining keys are different:
             *        - type -1: ESTOP. No additional fields
             *        - type 0: Test control module. No additional fields
             *        - type 1: Simple actuator controls. Takes in two additional fields:
             *          - refresh: bool: whether to render a refresh button
             *          - ids: array of ints: which simple actuator IDs to track
             *        - type 2: Stepper motor controls. Takes in the same arguments as type 1
             *        - type 3: Prompt display. No additional fields
             *        - type 128: Custom data display/log display: Takes no additional fields
             *        - types 0x90 - 0xC0: Sensor display. Any device class in this range can be used
             *          to create a sensor display module. This takes two fields:
             *          - abridged: bool: whether to display the sensors in abridged mode
             *          - ids: array of objects: which sensors to track. Each object has the format:
             *            - class: int: device class of sensor
             *            - ids: array of ints: which ids of the device class to track. At least 1 ID is required
             */

    // Helper function that resets and initializes everything based on the configuration file
    void TargetChooser::initWindows() {
        configName = targetconfig["name"].get<std::string>();
        interfName = interf->interfaceType();

        // Parse test list and pass to TestState
        std::map<uint8_t, std::string> tests;
        for(int i = 0; i < targetconfig["tests"].size(); i++) {
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
        for(int i = 0; i < targetconfig["devices"].size(); i++) {
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
                BoolSensors::getInstance()->setHardwareConfig(quals);
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
        for(int i = 0; i < targetconfig["windows"].size(); i++) {
            std::vector<WModule*> modules;

            // Iterate over the modules array
            for(int j = 0; j < targetconfig["windows"][i]["modules"].size(); j++) {
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
                case RCP_DEVCLASS_STEPPER: {
                    bool refresh = targetconfig["windows"][i]["modules"][j]["refresh"].get<bool>();
                    auto ids = targetconfig["windows"][i]["modules"][j]["ids"].get<std::set<int>>();

                    // Filter out any qualifiers that havent been configured in the devices section
                    auto filtered = allquals | std::views::filter([&type, &ids](const HardwareQualifier& q) {
                        return q.devclass == type && ids.contains(q.id);
                    });

                    // Determine correct module type and construct it
                    if(type == RCP_DEVCLASS_SIMPLE_ACTUATOR)
                        modules.push_back(
                            new SimpleActuatorViewer(std::set(filtered.begin(), filtered.end()), refresh));
                    else if(type == RCP_DEVCLASS_STEPPER)
                        modules.push_back(
                            new StepperViewer(std::set(filtered.begin(), filtered.end()), refresh));
                    else modules.push_back(new BoolSensorViewer(std::set(filtered.begin(), filtered.end()), refresh));

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
                    for(int k = 0; k < targetconfig["windows"][i]["modules"][j]["ids"].size(); k++) {
                        // Json's getting a little long lol
                        int devclass = targetconfig["windows"][i]["modules"][j]["ids"][k]["class"].get<int>();
                        auto ids = targetconfig["windows"][i]["modules"][j]["ids"][k]["ids"]
                            .get<std::set<int>>();

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
}
