#include "UI/StepperViewer.h"

namespace LRI::RCI {
    StepperViewer::StepperViewer(const std::set<HardwareQualifier>&& quals, bool refreshButton)
        : refreshButton(refreshButton) {
        for(const auto& qual : quals) {
            steppers[qual] = Steppers::getInstance()->getState(qual);
            inputs[qual] = Input();
        }
    }


    void StepperViewer::render() {
        ImDrawList* draw = ImGui::GetWindowDrawList();

        bool lockButtons = buttonTimer.timeSince() < BUTTON_DELAY;

        // Button for manually refreshing the states of all steppers
        if(refreshButton) {
            if(lockButtons) ImGui::BeginDisabled();
            if(ImGui::Button("Invalidate Cache and Refresh States")) {
                Steppers::getInstance()->refreshAll();
                buttonTimer.reset();
            }
            if(lockButtons) ImGui::EndDisabled();
            ImGui::Separator();
        }

        for(auto& [id, step] : steppers) {
            // Status square
            ImVec2 pos = ImGui::GetCursorScreenPos();
            ImU32 statusColor = step->stale ? STALE_COLOR : ENABLED_COLOR;
            const char* tooltip = step->stale ? "Stale Data" : "Current Data";
            draw->AddRectFilled(pos, pos + scale(STATUS_SQUARE_SIZE), statusColor);
            ImGui::Dummy(scale(STATUS_SQUARE_SIZE));
            if(ImGui::IsItemHovered()) ImGui::SetTooltip(tooltip);
            ImGui::SameLine();

            ImGui::Text("Stepper Motor %s (%d)", id.asString().c_str(), id.id);

            // Button for toggling how the inputted value will be interpreted
            ImGui::Text("Control Mode: ");
            for(const auto& [controlMode, strings] : BTN_NAMES) {
                bool activemode = inputs[id].mode == controlMode;
                if(activemode) {
                    ImGui::PushStyleColor(ImGuiCol_Button, REBECCA_PURPLE);
                    ImGui::PushStyleColor(ImGuiCol_ButtonActive, REBECCA_PURPLE);
                    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, REBECCA_PURPLE);
                }

                if(ImGui::Button((strings[0] + id.asString()).c_str())) inputs[id].mode = controlMode;

                if(activemode) ImGui::PopStyleColor(3);
                ImGui::SameLine();
            }

            // The actual control value
            ImGui::NewLine();
            ImGui::Text("Value: ");
            ImGui::SameLine();
            ImGui::SetNextItemWidth(scale(75));
            ImGui::InputFloat((" " + BTN_NAMES.at(inputs[id].mode)[1] + std::string("##inputval") +
                                  id.asString()).c_str(), &inputs[id].val);

            // The apply button actually sends the control value to the stepper motors
            if(lockButtons || step->stale) ImGui::BeginDisabled();
            ImGui::SameLine();
            if(ImGui::Button((std::string("Apply##") + id.asString()).c_str())) {
                Steppers::getInstance()->setState(id, inputs[id].mode, inputs[id].val);
                buttonTimer.reset();
            }

            if(lockButtons || step->stale) ImGui::EndDisabled();

            // Text for the current state of the stepper
            ImGui::Text("Current State: ");
            ImGui::Text("   Position: %.3f degrees", step->position);
            ImGui::Text("   Speed:    %.3f degrees/second", step->speed);

            ImGui::NewLine();
            ImGui::Separator();
        }
    }

    void StepperViewer::reset() {
        steppers.clear();
    }
}