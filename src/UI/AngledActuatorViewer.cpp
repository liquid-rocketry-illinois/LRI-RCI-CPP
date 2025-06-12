#include "UI/AngledActuatorViewer.h"

#include "imgui.h"

namespace LRI::RCI {
    AngledActuatorViewer::AngledActuatorViewer(const std::set<HardwareQualifier>& quals) {
        for(const auto& qual : quals) actuators[qual] = AngledActuators::getInstance()->getState(qual);
    }

    void AngledActuatorViewer::render() {
        ImGui::PushID("AngledActuatorViewer");
        ImGui::PushID(classid);

        // If an action has been taken in the last 1 second, lock buttons so users cant spam
        bool lockButton = buttonTimer.timeSince() < BUTTON_DELAY;

        // Iterate over each item being tracked
        for(const auto& [qual, data] : actuators) {
            // Display the name, current angle
            ImGui::Text("Actuator %s", qual.name.c_str());
            if(data != nullptr && data->size() > 0)
                ImGui::Text("Current angle: %.03f", data->at(data->size() - 1).data[0]);
            else ImGui::Text("Current angle: data not available");

            // Option to set the angle
            ImGui::Text("Set angle: ");
            ImGui::SameLine();
            ImGui::PushID(qual.asString().c_str());
            ImGui::SetNextItemWidth(scale(75));
            ImGui::InputFloat("##setpoint", &setpoints[qual]);
            ImGui::SameLine();
            ImGui::Text("degrees");
            ImGui::SameLine();

            // If we push the set button, communicate this over RCP
            if(lockButton) ImGui::BeginDisabled();
            if(ImGui::Button("Set")) {
                AngledActuators::getInstance()->setActuatorPos(qual, setpoints[qual]);
                buttonTimer.reset();
            }

            if(lockButton) ImGui::EndDisabled();

            ImGui::PopID();
        }

        ImGui::PopID();
        ImGui::PopID();
    }
} // namespace LRI::RCI
