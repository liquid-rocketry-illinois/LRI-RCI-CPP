#include "UI/MotorViewer.h"

#include "imgui.h"

#include "UI/gutils.h"
#include "hardware/hwctrl.h"

namespace LRI::RCI {
    MotorViewer::MotorViewer(const std::set<HardwareQualifier>& quals, bool refreshButton) :
        refreshButton(refreshButton) {
        for(const auto& qual : quals) {
            states[qual] = {hwctrl::getELog()->getActuatorFloatData(qual), 0};
        }
    }

    void MotorViewer::render() {
        ImGui::PushID("MotorViewer");
        ImGui::PushID(classid);

        bool lockButtons = buttonTimer.timeSince() < BUTTON_DELAY;

        bool disable = !hwctrl::isTargetReady() || hwctrl::getTestState() == RCP_TEST_RUNNING;
        if(disable) ImGui::BeginDisabled();

        if(refreshButton) {
            if(lockButtons) ImGui::BeginDisabled();
            if(ImGui::Button("Refresh All")) {
                for(const auto& [qual, d] : states) hwctrl::refresh(qual);
                buttonTimer.reset();
            }
            if(lockButtons) ImGui::EndDisabled();
            ImGui::Separator();
        }

        ImDrawList* draw = ImGui::GetWindowDrawList();

        for(auto& [qual, sdata] : states) {
            auto& [data, input] = sdata; // waow i love structured bindings
            ImGui::PushID(qual.asString().c_str());

            // Status square
            ImVec2 pos = ImGui::GetCursorScreenPos();
            ImU32 statusColor = data.values->empty() ? STALE_COLOR : ENABLED_COLOR;
            const char* tooltip = data.values->empty() ? "Stale Data" : "Current Data";
            draw->AddRectFilled(pos, pos + scale(STATUS_SQUARE_SIZE), statusColor);
            ImGui::Dummy(scale(STATUS_SQUARE_SIZE));
            if(ImGui::IsItemHovered()) ImGui::SetTooltip("%s", tooltip);
            ImGui::SameLine();

            ImGui::Text("Motor %s (%d)", qual.name.c_str(), qual.id);
            if(!data.values->empty()) ImGui::Text("Current Reported Speed: %f", data.values->back());
            else ImGui::Text("Current Reported Speed: 0");
            ImGui::Text("Set value: ");
            ImGui::SameLine();
            ImGui::SetNextItemWidth(75_sc);
            ImGui::InputFloat("##motorinput", &input);
            ImGui::SameLine();

            if(lockButtons || data.values->empty()) ImGui::BeginDisabled();
            if(ImGui::Button("Set")) {
                hwctrl::writeMotor(qual.id, input);
                buttonTimer.reset();
            }
            if(lockButtons || data.values->empty()) ImGui::EndDisabled();

            ImGui::NewLine();
            ImGui::Separator();
            ImGui::PopID();
        }

        if(disable) ImGui::EndDisabled();

        ImGui::PopID();
        ImGui::PopID();
    }
} // namespace LRI::RCI
