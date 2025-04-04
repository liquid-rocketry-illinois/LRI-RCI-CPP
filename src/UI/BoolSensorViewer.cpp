#include "UI/BoolSensorViewer.h"

#include "hardware/BoolSensor.h"

namespace LRI::RCI {
    int BoolSensorViewer::CLASSID = 0;

    BoolSensorViewer::BoolSensorViewer(const std::set<HardwareQualifier>& quals, bool refreshButton) :
        classid(CLASSID++), refreshButton(refreshButton) {
        for(const auto& qual : quals) {
            sensors[qual] = BoolSensors::getInstance()->getState(qual);
        }
    }

    void BoolSensorViewer::render() {
        ImGui::PushID("BoolSensorViewer");
        ImGui::PushID(classid);

        bool lockbutton = buttonTimer.timeSince() < BUTTON_DELAY;
        if(lockbutton) ImGui::BeginDisabled();

        if(refreshButton && ImGui::Button("Refresh")) {
            BoolSensors::getInstance()->refreshAll();
            buttonTimer.reset();
        }

        if(lockbutton) ImGui::EndDisabled();

        ImDrawList* draw = ImGui::GetWindowDrawList();

        for(const auto& [qual, state] : sensors) {
            ImGui::PushID(qual.asString().c_str());

            ImVec2 pos = ImGui::GetCursorScreenPos();
            ImU32 statusColor = !state->stale ? (state->open ? ENABLED_COLOR : DISABLED_COLOR) : STALE_COLOR;
            const char* tooltip = !state->stale ? (state->open ? "TRUE" : "FALSE") : "Stale Data";
            draw->AddRectFilled(pos, pos + scale(STATUS_SQUARE_SIZE), statusColor);
            ImGui::Dummy(scale(STATUS_SQUARE_SIZE));
            if(ImGui::IsItemHovered()) ImGui::SetTooltip(tooltip);
            ImGui::SameLine();
            ImGui::Text("Sensor %s (%d): ", qual.name.c_str(), qual.id);
            ImGui::SameLine();
            if(state->open) {
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


}
