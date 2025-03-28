#include "UI/TargetChooser.h"

#include <fstream>
#include <set>
#include <UI/RawViewer.h>
#include <UI/EStopViewer.h>
#include <UI/SensorViewer.h>
#include <UI/SolenoidViewer.h>
#include <UI/StepperViewer.h>
#include <UI/TestStateViewer.h>

#include "imgui.h"
#include "RCP_Host/RCP_Host.h"

#include "utils.h"
#include "RCP_Host_Impl.h"

#include "interfaces/VirtualPort.h"
#include "interfaces/COMPort.h"
#include "interfaces/TCPSocket.h"

namespace LRI::RCI {
    // This window always exists and is used to control the rest of the program
    TargetChooser::TargetChooser(ControlWindowlet* control)
        : control(control), interf(nullptr), pollingRate(25), chooser(nullptr), chosenConfig(0), chosenInterface(0) {
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
                    initWindows();
                }
            }
        }


        ImGui::PopID();
    }

    const RCP_Interface* TargetChooser::getInterface() const {
        return interf;
    }

    // Helper function that resets and initializes all windows
    void TargetChooser::initWindows() {
        // RawViewer::getInstance()->reset();
        // SensorViewer::getInstance()->reset();
        // SolenoidViewer::getInstance()->reset();
        // StepperViewer::getInstance()->reset();
        // TestStateViewer::getInstance()->reset();
        //
        // // These 3 can be shown regardless of the target
        // TestStateViewer::getInstance()->showWindow();
        // EStopViewer::getInstance()->showWindow();
        // RawViewer::getInstance()->showWindow();
        //
        // // Iterate through all the devices in the json and initialize the appropriate windows
        // std::set<SensorQualifier> sensors;
        // for(int i = 0; i < targetconfig["devices"].size(); i++) {
        //     auto devclass = static_cast<RCP_DeviceClass_t>(targetconfig["devices"][i]["devclass"].get<int>());
        //     std::vector<uint8_t> ids;
        //     std::vector<std::string> names;
        //
        //     if(targetconfig["devices"][i].contains("ids"))
        //         ids = targetconfig["devices"][i]["ids"].get<std::vector<uint8_t>>();
        //
        //     if(targetconfig["devices"][i].contains("names"))
        //         names = targetconfig["devices"][i]["names"].get<std::vector<std::string>>();
        //
        //     switch(devclass) {
        //         case RCP_DEVCLASS_SOLENOID: {
        //             std::map<uint8_t, std::string> sols;
        //
        //             if(ids.empty() && names.empty()) sols[0] = "Solenoid";
        //             else {
        //                 if(ids.size() != names.size()) break;
        //                 for(size_t j = 0; j < ids.size(); j++) sols[ids[j]] = names[j];
        //             }
        //
        //             SolenoidViewer::getInstance()->setHardwareConfig(sols);
        //             SolenoidViewer::getInstance()->showWindow();
        //             break;
        //         }
        //
        //         case RCP_DEVCLASS_STEPPER: {
        //             std::map<uint8_t, std::string> steps;
        //
        //             if(ids.empty() && names.empty()) steps[0] = "Stepper Motor";
        //             else {
        //                 if(ids.size() != names.size()) break;
        //                 for(size_t j = 0; j < ids.size(); j++) steps[ids[j]] = names[j];
        //             }
        //
        //             StepperViewer::getInstance()->setHardwareConfig(steps);
        //             StepperViewer::getInstance()->showWindow();
        //             break;
        //         }
        //
        //         case RCP_DEVCLASS_AM_PRESSURE:
        //         case RCP_DEVCLASS_AM_TEMPERATURE:
        //         case RCP_DEVCLASS_PRESSURE_TRANSDUCER:
        //         case RCP_DEVCLASS_RELATIVE_HYGROMETER:
        //         case RCP_DEVCLASS_LOAD_CELL:
        //         case RCP_DEVCLASS_POWERMON:
        //         case RCP_DEVCLASS_ACCELEROMETER:
        //         case RCP_DEVCLASS_GYROSCOPE:
        //         case RCP_DEVCLASS_MAGNETOMETER:
        //         case RCP_DEVCLASS_GPS: {
        //             if(ids.empty() && names.empty())
        //                 sensors.insert({
        //                                        .devclass = devclass,
        //                                        .id = 0,
        //                                        .name = devclassToString(devclass)
        //                                });
        //
        //             else {
        //                 if(ids.size() != names.size()) break;
        //                 for(size_t j = 0; j < ids.size(); j++)
        //                     sensors.insert({
        //                                            .devclass = devclass,
        //                                            .id = ids[j],
        //                                            .name = names[j]
        //                                    });
        //             }
        //
        //             break;
        //         }
        //
        //         default:
        //             break;
        //     }
        // }
        //
        // // If there is an actual sensor present, then display the sensors window
        // if(!sensors.empty()) {
        //     SensorViewer::getInstance()->setHardwareConfig(sensors);
        //     SensorViewer::getInstance()->showWindow();
        // }
    }
}
