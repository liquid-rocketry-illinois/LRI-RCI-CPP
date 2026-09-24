#include "UI/DiscreteActuatorViewer.h"

#include "UI/gutils.h"
#include "hardware/hwctrl.h"
#include "imgui.h"

// Module for viewing and controlling simple actuators
namespace LRI::RCI {
    // Add the qualifiers to track and their associated state pointer to the map
    DiscreteActuatorViewer::DiscreteActuatorViewer(const std::set<HardwareQualifier>& quals, const bool refreshButton) :
        refreshButton(refreshButton), evilMode(false), prevChordState(false) {
        for(const auto& qual : quals) {
            acts[qual] = hwctrl::getELog()->getActuatorUintData(qual);
        }
    }

    void DiscreteActuatorViewer::render() {
        bool chord = ImGui::GetIO().KeyCtrl && ImGui::GetIO().KeyAlt && ImGui::IsKeyDown(ImGuiKey_E);
        if(prevChordState != chord) {
            prevChordState = chord;
            if(chord) {
                evilMode = !evilMode;
            }
        }

        ImGui::PushID("SolenoidViewer");
        ImGui::PushID(classid);

        // If a test is running, lock the controls
        bool lockControls = !hwctrl::isTargetReady() || hwctrl::getTestState() == RCP_TEST_RUNNING;
        if(lockControls) ImGui::BeginDisabled();

        ImDrawList* draw = ImGui::GetWindowDrawList();

        bool lockButtons = buttonTimer.timeSince() < BUTTON_DELAY;
        if(lockButtons) ImGui::BeginDisabled();

        // A button to manually refresh the states of each solenoid
        if(refreshButton && ImGui::Button("Refresh All")) {
            for(const auto& [qual, data] : acts) hwctrl::refresh(qual);
            buttonTimer.reset();
        }
        if(lockButtons) ImGui::EndDisabled();
        ImGui::Separator();

        const auto now = std::chrono::system_clock::now();

        // Rendering each solenoid is simple. It consists of a status square, and a button to turn the solenoid
        // on and off
        for(const auto& [qual, data] : acts) {
            ImGui::PushID(qual.asString().c_str());

            // Status square
            ImVec2 pos = ImGui::GetCursorScreenPos();
            ImU32 statusColor = data.times->empty() ? STALE_COLOR : ENABLED_COLOR;
            draw->AddRectFilled(pos, pos + scale(STATUS_SQUARE_SIZE), statusColor);
            ImGui::Dummy(scale(STATUS_SQUARE_SIZE));
            if(ImGui::IsItemHovered()) {
                if(statusColor == STALE_COLOR) ImGui::SetTooltip("No Data");
                else {
                    auto timePassed =
                        std::chrono::duration_cast<std::chrono::nanoseconds>(now - data.times->back().htime) /
                        static_cast<float>(10e9);
                    ImGui::SetTooltip("Updated %.3f seconds ago", timePassed.count());
                }
            }

            // Solenoid name and ID
            ImGui::SameLine();
            ImGui::Text("%s (%d)", qual.name.c_str(), qual.id);

            ImGui::SameLine();

            // Control button
            if(statusColor == STALE_COLOR || lockButtons) ImGui::BeginDisabled();
            // For now, we assume discrete actuators are equivalent to SimpleActuators from <1.4.0
            bool currentState = !data.times->empty() && data.values->back() > 0;
            const char* text = nullptr;
            if(evilMode) text = currentState ? "OFF (how evil)" : "ON (how evil)";
            else text = currentState ? "OFF" : "ON";
            if(ImGui::Button(text)) {
                hwctrl::writeDA(qual.id, currentState ? 0 : 1);
                buttonTimer.reset();
            }
            if(statusColor == STALE_COLOR || lockButtons) ImGui::EndDisabled();

            ImGui::PopID();
        }

        if(lockControls) ImGui::EndDisabled();

        ImGui::PopID();
        ImGui::PopID();
    }
} // namespace LRI::RCI
