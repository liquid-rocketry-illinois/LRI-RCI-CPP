#include "UI/BoolSensorViewer.h"

#include "hardware/hwctrl.h"

#include "UI/gutils.h"

// Module for viewing the BoolSensor states
namespace LRI::RCI {
    // Add the qualifiers to track and their associated state pointer to the map
    BoolSensorViewer::BoolSensorViewer(const std::set<HardwareChannel>& quals, bool refreshButton) :
        refreshButton(refreshButton) {
        for(const auto& qual : quals) sensors[qual] = {hwctrl::getELog()->getChannelUintData(qual), true};
    }

    void BoolSensorViewer::render() {
        ImGui::PushID("BoolSensorViewer");
        ImGui::PushID(classid);

        // If an action has been taken in the last 1 second, lock buttons so users cant spam
        bool lockbutton = buttonTimer.timeSince() < BUTTON_DELAY;
        if(lockbutton) ImGui::BeginDisabled();

        // Request refresh of all sensors
        if(refreshButton) {
            if(ImGui::Button("Refresh")) {
                for(const auto& [qual, state] : sensors) hwctrl::refresh(qual);
                buttonTimer.reset();
            }
            ImGui::Separator();
        }

        if(lockbutton) ImGui::EndDisabled();

        ImDrawList* draw = ImGui::GetWindowDrawList();

        // For each sensor, draw its little display thing
        for(const auto& [qual, state] : sensors) {
            ImGui::PushID(qual.asString().c_str());

            // Draw the little status square
            ImVec2 pos = ImGui::GetCursorScreenPos();

            ImU32 statusColor = state.stale || state.data.values->empty()
                ? STALE_COLOR
                : (state.data.values->back() ? ENABLED_COLOR : DISABLED_COLOR);

            // Comparisons with statusColor are made since its just an integer so comparisons are fast
            const char* tooltip =
                statusColor == STALE_COLOR ? "Stale Data" : (statusColor == ENABLED_COLOR ? "TRUE" : "FALSE");
            draw->AddRectFilled(pos, pos + scale(STATUS_SQUARE_SIZE), statusColor);
            ImGui::Dummy(scale(STATUS_SQUARE_SIZE));
            if(ImGui::IsItemHovered()) ImGui::SetTooltip("%s", tooltip);

            // Display the name of the sensor and its value
            ImGui::SameLine();
            ImGui::Text("%s (%d):", qual.name.c_str(), qual.id);
            ImGui::SameLine();
            if(statusColor == ENABLED_COLOR) {
                ImGui::PushStyleColor(ImGuiCol_Text, ENABLED_COLOR);
                ImGui::Text("TRUE");
            }

            else {
                ImGui::PushStyleColor(ImGuiCol_Text, DISABLED_COLOR);
                ImGui::Text("FALSE");
            }

            ImGui::PopStyleColor();

            ImGui::PopID();
        }

        ImGui::PopID();
        ImGui::PopID();
    }


} // namespace LRI::RCI
