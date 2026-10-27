#include "UI/AngledActuatorViewer.h"

#include "imgui.h"

#include "UI/gutils.h"
#include "hardware/hwctrl.h"

namespace LRI::RCI {
    AngledActuatorViewer::AngledActuatorViewer(const std::set<HardwareQualifier>& quals, bool refreshButton) :
        refreshButton(refreshButton) {
        for(const auto& qual : quals) {
            HardwareChannel tempch = {qual, 0};
            actuators[qual] = hwctrl::getELog()->getChannelFloatData(tempch);
        }
    }

    void AngledActuatorViewer::render() {
        ImGui::PushID("AngledActuatorViewer");
        ImGui::PushID(classid);

        // If an action has been taken in the last 1 second, lock buttons so users cant spam
        const bool lockButton = buttonTimer.timeSince() < BUTTON_DELAY;

        if(lockButton) ImGui::BeginDisabled();
        if(refreshButton && ImGui::Button("Refresh All")) {
            for(const auto& [qual, data] : actuators) hwctrl::refresh(qual);
            buttonTimer.reset();
        }
        if(lockButton) ImGui::EndDisabled();

        // Iterate over each item being tracked
        for(const auto& [qual, data] : actuators) {
            // Display the name, current angle
            ImGui::Text("Actuator %s", qual.name.c_str());
            if(!data.values->empty())
                ImGui::Text("Current angle: %.03f", data.values->back());
            else ImGui::Text("Current angle: data not available");

            // Option to set the angle
            ImGui::Text("Set angle: ");
            ImGui::SameLine();
            ImGui::PushID(qual.asString().c_str());
            ImGui::SetNextItemWidth(75_sc);
            ImGui::InputFloat("##setpoint", &setpoints[qual]);
            ImGui::SameLine();
            ImGui::Text("degrees ");
            ImGui::SameLine();

            // If we push the set button, communicate this over RCP
            if(lockButton) ImGui::BeginDisabled();
            if(ImGui::Button("Set")) {
                hwctrl::writeAA(qual.id, setpoints[qual]);
                buttonTimer.reset();
            }

            if(lockButton) ImGui::EndDisabled();

            ImGui::PopID();
            ImGui::Separator();
        }

        ImGui::PopID();
        ImGui::PopID();
    }
} // namespace LRI::RCI
