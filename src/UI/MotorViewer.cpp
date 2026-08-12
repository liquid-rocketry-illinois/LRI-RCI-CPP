#include "UI/MotorViewer.h"

#include "imgui.h"

#include "hardware/TestState.h"
#include "UI/gutils.h"

namespace LRI::RCI {
    MotorViewer::MotorViewer(const std::set<HardwareQualifier>& quals, bool refreshButton) :
        refreshButton(refreshButton) {
        for(const auto& qual : quals) {
            const auto* motor = Motors::getState(qual);
            if(motor == nullptr) continue;
            states[qual] = motor;
            inputs[qual] = 0;
        }
    }

    void MotorViewer::render() {
        ImGui::PushID("MotorViewer");
        ImGui::PushID(classid);

        bool lockButtons = buttonTimer.timeSince() < BUTTON_DELAY;

        if(!TestState::getInited() || TestState::getState() == RCP_TEST_RUNNING) ImGui::BeginDisabled();

        if(refreshButton) {
            if(lockButtons) ImGui::BeginDisabled();
            if(ImGui::Button("Refresh All")) {
                Motors::refreshAll();
                buttonTimer.reset();
            }
            if(lockButtons) ImGui::EndDisabled();
            ImGui::Separator();
        }

        ImDrawList* draw = ImGui::GetWindowDrawList();

        for(auto& [id, motor] : states) {
            ImGui::PushID(id.asString().c_str());

            // Status square
            ImVec2 pos = ImGui::GetCursorScreenPos();
            ImU32 statusColor = motor->empty() ? STALE_COLOR : ENABLED_COLOR;
            const char* tooltip = motor->empty() ? "Stale Data" : "Current Data";
            draw->AddRectFilled(pos, pos + scale(STATUS_SQUARE_SIZE), statusColor);
            ImGui::Dummy(scale(STATUS_SQUARE_SIZE));
            if(ImGui::IsItemHovered()) ImGui::SetTooltip(tooltip);
            ImGui::SameLine();

            ImGui::Text("Motor %s (%d)", id.name.c_str(), id.id);
            if(!motor->empty()) ImGui::Text("Current Reported Speed: %f", motor->at(motor->size() - 1));
            else ImGui::Text("Current Reported Speed: % 6.1f", 0);
            ImGui::Text("Set value: ");
            ImGui::SameLine();
            ImGui::SetNextItemWidth(75_sc);
            ImGui::InputFloat("##motorinput", &inputs[id]);
            ImGui::SameLine();

            if(lockButtons || motor->empty()) ImGui::BeginDisabled();
            if(ImGui::Button("Set")) {
                Motors::setState(id, inputs[id]);
                buttonTimer.reset();
            }
            if(lockButtons || motor->empty()) ImGui::EndDisabled();

            ImGui::NewLine();
            ImGui::Separator();
            ImGui::PopID();
        }

        if(!TestState::getInited() || TestState::getState() == RCP_TEST_RUNNING) ImGui::EndDisabled();

        ImGui::PopID();
        ImGui::PopID();
    }
} // namespace LRI::RCI
