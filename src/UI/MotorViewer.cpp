#include "UI/MotorViewer.h"

#include "imgui.h"

#include "hardware/TestState.h"

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

        bool lockButtons = buttonTimer.timeSince() > BUTTON_DELAY;

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
            ImU32 statusColor = motor->stale ? STALE_COLOR : ENABLED_COLOR;
            const char* tooltip = motor->stale ? "Stale Data" : "Current Data";
            draw->AddRectFilled(pos, pos + scale(STATUS_SQUARE_SIZE), statusColor);
            ImGui::Dummy(scale(STATUS_SQUARE_SIZE));
            if(ImGui::IsItemHovered()) ImGui::SetTooltip(tooltip);
            ImGui::SameLine();

            ImGui::Text("Motor %s (%d)", id.name.c_str(), id.id);
            ImGui::Text("Current Reported Speed: %f", motor->value);
            ImGui::Text("Set value: ");
            ImGui::SameLine();
            ImGui::SetNextItemWidth(scale(75));
            ImGui::InputFloat("##motorinput", &inputs[id]);
            ImGui::SameLine();

            if(lockButtons || motor->stale) ImGui::BeginDisabled();
            if(ImGui::Button("Set")) {
                Motors::setState(id, inputs[id]);
                buttonTimer.reset();
            }
            if(lockButtons || motor->stale) ImGui::EndDisabled();

            ImGui::NewLine();
            ImGui::Separator();
            ImGui::PopID();
        }

        if(!TestState::getInited() || TestState::getState() == RCP_TEST_RUNNING) ImGui::EndDisabled();

        ImGui::PopID();
        ImGui::PopID();
    }
} // namespace LRI::RCI
