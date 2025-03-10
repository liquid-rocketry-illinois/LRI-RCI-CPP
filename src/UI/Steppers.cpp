#include "UI/Steppers.h"

#include <ctime>

namespace LRI::RCI {
    Steppers* Steppers::instance;

    // Maps stepper control modes to the correct name and unit
    const std::map<uint8_t, std::vector<std::string>> Steppers::BTN_NAMES = {
            {RCP_STEPPER_ABSOLUTE_POS_CONTROL, {"Absolute Positioning##", "degrees"}},
            {RCP_STEPPER_RELATIVE_POS_CONTROL, {"Relative Positioning##", "degrees"}},
            {RCP_STEPPER_SPEED_CONTROL,        {"Velocity Control##",     "degrees/s"}}
    };

    Steppers* Steppers::getInstance() {
        if(instance == nullptr) instance = new Steppers();
        return instance;
    }

    void Steppers::render() {
        ImGui::SetNextWindowPos(scale(ImVec2(675, 375)), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(scale(ImVec2(550, 400)), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowCollapsed(true, ImGuiCond_FirstUseEver);
        if(ImGui::Begin("Manual Stepper Motor Control")) {
            ImDrawList* draw = ImGui::GetWindowDrawList();

            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1, 0, 0, 1));
            ImGui::PushTextWrapPos(0);
            ImGui::Text("WARNING: This panel is for manual control only. Stepper states may be stale.");
            ImGui::PopTextWrapPos();
            ImGui::PopStyleColor();

            ImGui::NewLine();
            bool lockButtons = buttonTimer.timeSince() < BUTTON_DELAY;

            // Button for manually refreshing the states of all steppers
            if(lockButtons) ImGui::BeginDisabled();
            if(ImGui::Button("Invalidate Cache and Refresh States")) {
                for(auto& [id, step] : steppers) {
                    RCP_requestStepperRead(id);
                    step.stale = true;
                }

                buttonTimer.reset();
            }
            if(lockButtons) ImGui::EndDisabled();

            ImGui::Separator();

            for(auto& [id, step] : steppers) {
                // Status square
                ImVec2 pos = ImGui::GetCursorScreenPos();
                ImU32 statusColor = step.stale ? STALE_COLOR : ENABLED_COLOR;
                const char* tooltip = step.stale ? "Stale Data" : "Current Data";
                draw->AddRectFilled(pos, pos + scale(STATUS_SQUARE_SIZE), statusColor);
                ImGui::Dummy(scale(STATUS_SQUARE_SIZE));
                if(ImGui::IsItemHovered()) ImGui::SetTooltip(tooltip);
                ImGui::SameLine();

                ImGui::Text("Stepper Motor %s (%d)", step.name.c_str(), id);

                // Button for toggling how the inputted value will be interpreted
                ImGui::Text("Control Mode: ");
                for(const auto& [controlMode, strings] : BTN_NAMES) {
                    bool activemode = step.controlMode == controlMode;
                    if(activemode) {
                        ImGui::PushStyleColor(ImGuiCol_Button, REBECCA_PURPLE);
                        ImGui::PushStyleColor(ImGuiCol_ButtonActive, REBECCA_PURPLE);
                        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, REBECCA_PURPLE);
                    }

                    if(ImGui::Button((strings[0] + std::to_string(id)).c_str())) step.controlMode = controlMode;

                    if(activemode) ImGui::PopStyleColor(3);
                    ImGui::SameLine();
                }

                // The actual control value
                ImGui::NewLine();
                ImGui::Text("Value: ");
                ImGui::SameLine();
                ImGui::SetNextItemWidth(scale(75));
                ImGui::InputFloat(
                        (" " + BTN_NAMES.at(step.controlMode)[1] + std::string("##inputval") +
                         std::to_string(id)).c_str(),
                        &step.controlVal);

                // The apply button actually sends the control value to the stepper motors
                if(lockButtons || step.stale) ImGui::BeginDisabled();
                ImGui::SameLine();
                if(ImGui::Button((std::string("Apply##") + std::to_string(id)).c_str())) {
                    RCP_sendStepperWrite(id, step.controlMode, &step.controlVal);
                    buttonTimer.reset();
                }

                if(lockButtons || step.stale) ImGui::EndDisabled();

                // Text for the current state of the stepper
                ImGui::Text("Current State: ");
                ImGui::Text("   Position: %.3f degrees", static_cast<float>(step.position));
                ImGui::Text("   Speed:    %.3f degrees/second", static_cast<float>(step.speed));

                ImGui::NewLine();
                ImGui::Separator();
            }
        }

        ImGui::End();
    }

    void Steppers::reset() {
        steppers.clear();
    }

    void Steppers::setHardwareConfig(const std::map<uint8_t, std::string>& ids) {
        reset();
        for(const auto& [id, name] : ids) {
            Stepper s{
                    .controlMode = RCP_STEPPER_ABSOLUTE_POS_CONTROL,
                    .position = 0,
                    .speed = 0,
                    .stale = true,
                    .name = name,
                    .controlVal = 0
            };
            steppers[id] = s;
            RCP_requestStepperRead(id);
        }
    }

    void Steppers::receiveRCPUpdate(const RCP_TwoFloat& state) {
        Stepper& s = steppers[state.ID];
        s.stale = false;
        s.position = state.data[0];
        s.speed = state.data[1];
    }
}
