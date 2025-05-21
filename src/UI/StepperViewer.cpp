#include "UI/StepperViewer.h"
#include "utils.h"
#include "hardware/TestState.h"

// Module for viewing steppers
namespace LRI::RCI {
    // Add the qualifiers to track and their associated state pointer to the map
    StepperViewer::StepperViewer(const std::set<HardwareQualifier>&& quals, bool refreshButton) :
        refreshButton(refreshButton) {
        for(const auto& qual : quals) {
            steppers[qual] = Steppers::getInstance()->getState(qual);
            inputs[qual] = Input();
        }
    }

    void StepperViewer::render() {
        ImGui::PushID("StepperViewer");
        ImGui::PushID(classid);

        // If a test is running, lock controls
        if(!TestState::getInited() || TestState::getInstance()->getState() == RCP_TEST_RUNNING) ImGui::BeginDisabled();

        ImDrawList* draw = ImGui::GetWindowDrawList();

        bool lockButtons = buttonTimer.timeSince() < BUTTON_DELAY;

        // Button for manually refreshing the states of all steppers
        if(refreshButton) {
            if(lockButtons) ImGui::BeginDisabled();
            if(ImGui::Button("Refresh All")) {
                Steppers::getInstance()->refreshAll();
                buttonTimer.reset();
            }
            if(lockButtons) ImGui::EndDisabled();
            ImGui::Separator();
        }

        for(auto& [id, step] : steppers) {
            ImGui::PushID(id.asString().c_str());

            // Status square
            ImVec2 pos = ImGui::GetCursorScreenPos();
            ImU32 statusColor = step->stale ? STALE_COLOR : ENABLED_COLOR;
            const char* tooltip = step->stale ? "Stale Data" : "Current Data";
            draw->AddRectFilled(pos, pos + scale(STATUS_SQUARE_SIZE), statusColor);
            ImGui::Dummy(scale(STATUS_SQUARE_SIZE));
            if(ImGui::IsItemHovered()) ImGui::SetTooltip(tooltip);
            ImGui::SameLine();

            ImGui::Text("Stepper Motor %s (%d)", id.name.c_str(), id.id);

            // Button for toggling how the inputted value will be interpreted
            ImGui::Text("Control Mode: ");
            for(const auto& [controlMode, strings] : BTN_NAMES) {
                bool activemode = inputs[id].mode == controlMode;
                if(activemode) {
                    ImGui::PushStyleColor(ImGuiCol_Button, REBECCA_PURPLE);
                    ImGui::PushStyleColor(ImGuiCol_ButtonActive, REBECCA_PURPLE);
                    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, REBECCA_PURPLE);
                }

                if(ImGui::Button(strings[0])) inputs[id].mode = controlMode;

                if(activemode) ImGui::PopStyleColor(3);
                ImGui::SameLine();
            }

            // The actual control value
            ImGui::NewLine();
            ImGui::Text("Value: ");
            ImGui::SameLine();
            ImGui::SetNextItemWidth(scale(75));
            ImGui::InputFloat(BTN_NAMES.at(inputs[id].mode)[1], &inputs[id].val);

            // The apply button actually sends the control value to the stepper motors
            if(lockButtons || step->stale) ImGui::BeginDisabled();
            ImGui::SameLine();
            if(ImGui::Button("Apply")) {
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
            ImGui::PopID();
        }

        if(!TestState::getInited() || TestState::getInstance()->getState() == RCP_TEST_RUNNING) ImGui::EndDisabled();
        ImGui::PopID();
        ImGui::PopID();
    }

    const std::map<RCP_StepperControlMode, std::vector<const char*>> StepperViewer::BTN_NAMES{
        {RCP_STEPPER_ABSOLUTE_POS_CONTROL, {"Absolute Positioning##", " degrees###input"}},
        {RCP_STEPPER_RELATIVE_POS_CONTROL, {"Relative Positioning##", " degrees###input"}},
        {RCP_STEPPER_SPEED_CONTROL, {"Velocity Control##", " degrees/s###input"}},
    };
}
