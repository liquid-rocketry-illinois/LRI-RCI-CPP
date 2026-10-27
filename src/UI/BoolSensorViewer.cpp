#include "UI/BoolSensorViewer.h"

#include "hardware/hwctrl.h"

#include "UI/gutils.h"

// Module for viewing the BoolSensor states
namespace LRI::RCI {
    // Add the qualifiers to track and their associated state pointer to the map
    BoolSensorViewer::BoolSensorViewer(const std::set<HardwareChannel>& quals, bool refreshButton) :
        refreshButton(refreshButton) {
        for(const auto& qual : quals) sensors[qual] = hwctrl::getELog()->getChannelUintData(qual);
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
                for(const auto& [qual, data] : sensors) hwctrl::refresh(qual);
                buttonTimer.reset();
            }
            ImGui::Separator();
        }

        if(lockbutton) ImGui::EndDisabled();

        ImDrawList* draw = ImGui::GetWindowDrawList();

        const auto now = std::chrono::system_clock::now();

        // For each sensor, draw its little display thing
        for(const auto& [qual, data] : sensors) {
            ImGui::PushID(qual.asString().c_str());

            // Draw the little status square
            ImVec2 pos = ImGui::GetCursorScreenPos();

            ImU32 statusColor =
                data.values->empty() ? STALE_COLOR : (data.values->back() ? ENABLED_COLOR : DISABLED_COLOR);

            // Comparisons with statusColor are made since its just an integer so comparisons are fast
            draw->AddRectFilled(pos, pos + scale(STATUS_SQUARE_SIZE), statusColor);
            ImGui::Dummy(scale(STATUS_SQUARE_SIZE));
            if(ImGui::IsItemHovered()) {
                if(statusColor == STALE_COLOR) ImGui::SetTooltip("No Data");
                else {
                    auto timePassed =
                        std::chrono::duration_cast<std::chrono::nanoseconds>(now - tpFromHt(data.times->back().htime)) /
                        static_cast<float>(10e9);
                    ImGui::SetTooltip("Updated %.3f seconds ago", timePassed.count());
                }
            }

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
