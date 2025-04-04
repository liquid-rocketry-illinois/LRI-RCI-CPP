#include "UI/TargetChooser.h"

#include <fstream>
#include <set>

#include "imgui.h"
#include "RCP_Host/RCP_Host.h"

#include "utils.h"
#include "RCP_Host_Impl.h"

#include "interfaces/VirtualPort.h"
#include "interfaces/COMPort.h"
#include "interfaces/TCPSocket.h"

#include "hardware/SimpleActuators.h"
#include "hardware/Steppers.h"
#include "hardware/Sensors.h"

#include "UI/EStopViewer.h"
#include "UI/PromptViewer.h"
#include "UI/RawViewer.h"
#include "UI/SensorViewer.h"
#include "UI/SimpleActuatorViewer.h"
#include "UI/StepperViewer.h"
#include "UI/TestStateViewer.h"

namespace LRI::RCI {
    // This window always exists and is used to control the rest of the program
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
                    delete chooser;
                    chooser = nullptr;
                    interf = _interf;
                    RCP_init(callbacks);
                    RCP_setChannel(RCP_CH_ZERO);
                    std::ifstream config(targetpaths[chosenConfig]);
                    targetconfig = nlohmann::json::parse(config);
                    control->interf = interf;
                    control->inipath = targetpaths[chosenConfig] + ".ini";
                    if(std::filesystem::exists(targetpaths[chosenConfig] + ".ini")) iniFilePath.path = control->inipath;

                    initWindows();
                }
            }
        }

        ImGui::PopID();
    }

    // Helper function that resets and initializes all windows
    void TargetChooser::initWindows() {
        configName = targetconfig["name"].get<std::string>();
        interfName = interf->interfaceType();
        std::set<HardwareQualifier> allquals;
        std::set<HardwareQualifier> sensors;

        for(int i = 0; i < targetconfig["devices"].size(); i++) {
            auto devclass = static_cast<RCP_DeviceClass>(targetconfig["devices"][i]["devclass"].get<int>());
            std::vector<uint8_t> ids = targetconfig["devices"][i]["ids"].get<std::vector<uint8_t>>();
            std::vector<std::string> names = targetconfig["devices"][i]["names"].get<std::vector<std::string>>();
            if(ids.empty() || ids.size() != names.size()) continue;
            std::set<HardwareQualifier> quals;
            for(size_t j = 0; j < ids.size(); j++) quals.insert(HardwareQualifier{devclass, ids[j], names[j]});
            allquals.insert(quals.begin(), quals.end());

            switch(devclass) {
            case RCP_DEVCLASS_STEPPER:
                Steppers::getInstance()->setHardwareConfig(quals);
                break;

            case RCP_DEVCLASS_SIMPLE_ACTUATOR:
                SimpleActuators::getInstance()->setHardwareConfig(quals);

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

        Sensors::getInstance()->setHardwareConfig(sensors);

        for(int i = 0; i < targetconfig["windows"].size(); i++) {
            std::vector<WModule*> modules;

            for(int j = 0; j < targetconfig["windows"][i]["modules"].size(); j++) {
                int type = targetconfig["windows"][i]["modules"][j]["type"].get<int>();

                switch(type) {
                case -1:
                    modules.push_back(new EStopViewer());
                    break;

                case RCP_DEVCLASS_TEST_STATE:
                    modules.push_back(new TestStateViewer());
                    break;

                case RCP_DEVCLASS_SIMPLE_ACTUATOR:
                case RCP_DEVCLASS_STEPPER: {
                    bool refresh = targetconfig["windows"][i]["modules"][j]["refresh"].get<bool>();
                    std::set<int> ids = targetconfig["windows"][i]["modules"][j]["ids"].get<std::set<int>>();
                    auto filtered = allquals | std::views::filter([&ids](const HardwareQualifier& q) {
                        return q.devclass == RCP_DEVCLASS_SIMPLE_ACTUATOR && ids.contains(q.id);
                    });
                    if(type == 1)
                        modules.push_back(
                            new SimpleActuatorViewer(std::set(filtered.begin(), filtered.end()), refresh));
                    else modules.push_back(new StepperViewer(std::set(filtered.begin(), filtered.end()), refresh));

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
                    bool abridged = targetconfig["windows"][i]["modules"][j]["abridged"].get<bool>();
                    std::set<HardwareQualifier> quals;

                    for(int k = 0; k < targetconfig["windows"][i]["modules"][j]["ids"].size(); k++) {
                        // Json's getting a little long lol
                        int devclass = targetconfig["windows"][i]["modules"][j]["ids"][k]["class"].get<int>();
                        std::set<int> ids = targetconfig["windows"][i]["modules"][j]["ids"][k]["ids"]
                            .get<std::set<int>>();
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

            new Windowlet(targetconfig["windows"][i]["title"].get<std::string>(), modules);
        }
    }
}
