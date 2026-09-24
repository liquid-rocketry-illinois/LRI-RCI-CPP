#include "UI/StepperViewer.h"

#include "UI/gutils.h"
#include "hardware/hwctrl.h"

// Module for viewing steppers
namespace LRI::RCI {
    // Add the qualifiers to track and their associated state pointer to the map
    StepperViewer::StepperViewer(const std::set<HardwareQualifier>& quals, bool refreshButton) :
        refreshButton(refreshButton) {
        for(const auto& qual : quals) {
            steppers[qual] = {hwctrl::getELog()->getAllChannels(qual)};
        }
    }

    void StepperViewer::render() {
        ImGui::PushID("StepperViewer");
        ImGui::PushID(classid);

        // If a test is running, lock controls
        bool testLock = !hwctrl::isTargetReady() || hwctrl::getTestState() == RCP_TEST_RUNNING;
        if(testLock) ImGui::BeginDisabled();

        ImDrawList* draw = ImGui::GetWindowDrawList();

        bool timeLock = buttonTimer.timeSince() < BUTTON_DELAY;

        // Button for manually refreshing the states of all steppers
        if(refreshButton) {
            if(timeLock) ImGui::BeginDisabled();
            if(ImGui::Button("Refresh All")) {
                for(const auto& [qual, state] : steppers) hwctrl::refresh(qual);
                buttonTimer.reset();
            }
            if(timeLock) ImGui::EndDisabled();
            ImGui::Separator();
        }

        const auto now = std::chrono::system_clock::now();

        for(auto& [qual, state] : steppers) {
            ImGui::PushID(qual.asString().c_str());

            // Status square
            ImVec2 pos = ImGui::GetCursorScreenPos();
            ImU32 statusColor = state.data[0].times->empty() ? STALE_COLOR : ENABLED_COLOR;
            draw->AddRectFilled(pos, pos + scale(STATUS_SQUARE_SIZE), statusColor);
            ImGui::Dummy(scale(STATUS_SQUARE_SIZE));
            if(ImGui::IsItemHovered()) {
                if(statusColor == STALE_COLOR) ImGui::SetTooltip("No Data");
                else {
                    auto timePassed =
                        std::chrono::duration_cast<std::chrono::nanoseconds>(now - state.data[0].times->back().htime) /
                        static_cast<float>(10e9);
                    ImGui::SetTooltip("Updated %.3f seconds ago", timePassed.count());
                }
            }

            ImGui::SameLine();
            ImGui::Text("Stepper Motor %s (%d)", qual.name.c_str(), qual.id);

            // Button for toggling how the inputted value will be interpreted
            ImGui::Text("Control Mode: ");
            for(const auto& [controlMode, strings] : BTN_NAMES) {
                bool activemode = state.mode == controlMode;
                if(activemode) {
                    ImGui::PushStyleColor(ImGuiCol_Button, REBECCA_PURPLE);
                    ImGui::PushStyleColor(ImGuiCol_ButtonActive, REBECCA_PURPLE);
                    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, REBECCA_PURPLE);
                }

                if(ImGui::Button(strings[0])) state.mode = controlMode;

                if(activemode) ImGui::PopStyleColor(3);
                ImGui::SameLine();
            }

            // The actual control value
            ImGui::NewLine();
            ImGui::Text("Value: ");
            ImGui::SameLine();
            ImGui::SetNextItemWidth(75_sc);
            ImGui::InputFloat(BTN_NAMES.at(state.mode)[1], &state.val);

            // The apply button actually sends the control value to the stepper motors
            if(timeLock || statusColor == STALE_COLOR) ImGui::BeginDisabled();
            ImGui::SameLine();
            if(ImGui::Button("Apply")) {
                hwctrl::writeStepper(qual.id, state.mode, state.val);
                buttonTimer.reset();
            }

            if(timeLock || statusColor == STALE_COLOR) ImGui::EndDisabled();

            // Text for the current state of the stepper
            ImGui::Text("Current State: ");
            if(statusColor == STALE_COLOR) {
                ImGui::Text("   Position: 0.000 degrees");
                ImGui::Text("   Speed:    0.000 degrees/second");
            }

            else {
                ImGui::Text("   Position: %.3f degrees", state.data[0].values->back());
                ImGui::Text("   Speed:    %.3f degrees/second", state.data[1].values->back());
            }

            ImGui::NewLine();
            ImGui::Separator();
            ImGui::PopID();
        }

        if(testLock) ImGui::EndDisabled();
        ImGui::PopID();
        ImGui::PopID();
    }

    const std::map<RCP_StepperControlMode, std::vector<const char*>> StepperViewer::BTN_NAMES{
        {RCP_STEPPER_ABSOLUTE_POS_CONTROL, {"Absolute Positioning##", " degrees###input"}},
        {RCP_STEPPER_RELATIVE_POS_CONTROL, {"Relative Positioning##", " degrees###input"}},
        {RCP_STEPPER_SPEED_CONTROL, {"Velocity Control##", " degrees/s###input"}},
    };
} // namespace LRI::RCI
