#include "UI/SimpleActuatorViewer.h"

#include "imgui.h"
#include "utils.h"
#include "RCP_Host/RCP_Host.h"
#include "hardware/TestState.h"

namespace LRI::RCI {
    int SimpleActuatorViewer::CLASSID = 0;

    SimpleActuatorViewer::SimpleActuatorViewer(const std::set<HardwareQualifier>&& quals, const bool refreshButton) :
        classid(CLASSID++), refreshButton(refreshButton) {
        for(const auto& sol : quals) {
            sols[sol] = SimpleActuators::getInstance()->getState(sol);
        }
    }

    void SimpleActuatorViewer::render() {
        ImGui::PushID("SolenoidViewer");
        ImGui::PushID(classid);
        if(!TestState::getInited()) ImGui::BeginDisabled();

        ImDrawList* draw = ImGui::GetWindowDrawList();

        bool lockButtons = buttonTimer.timeSince() < BUTTON_DELAY;
        if(lockButtons) ImGui::BeginDisabled();

        // A button to manually refresh the states of each solenoid
        if(refreshButton && ImGui::Button("Refresh All")) {
            SimpleActuators::getInstance()->refreshAll();
            buttonTimer.reset();
        }
        if(lockButtons) ImGui::EndDisabled();
        ImGui::Separator();

        // Rendering each solenoid is simple. It consists of a status square, and a button to turn the solenoid
        // on and off
        for(const auto& [id, state] : sols) {
            ImGui::PushID(id.asString().c_str());

            ImVec2 pos = ImGui::GetCursorScreenPos();
            ImU32 statusColor = !state->stale ? (state->open ? ENABLED_COLOR : DISABLED_COLOR) : STALE_COLOR;
            const char* tooltip = !state->stale ? (state->open ? "ON" : "OFF") : "Stale Data";
            draw->AddRectFilled(pos, pos + scale(STATUS_SQUARE_SIZE), statusColor);
            ImGui::Dummy(scale(STATUS_SQUARE_SIZE));
            if(ImGui::IsItemHovered()) ImGui::SetTooltip(tooltip);
            ImGui::SameLine();
            ImGui::Text("Solenoid %s (%d)", id.name.c_str(), id.id);

            ImGui::SameLine();

            if(lockButtons || !state->stale) ImGui::BeginDisabled();
            if(ImGui::Button(!state->open ? "ON" : "OFF")) {
                SimpleActuators::getInstance()->setActuatorState(
                    id, state->open ? RCP_SIMPLE_ACTUATOR_OFF : RCP_SIMPLE_ACTUATOR_ON);
                buttonTimer.reset();
            }
            if(lockButtons || !state->stale) ImGui::EndDisabled();

            ImGui::PopID();
        }

        if(!TestState::getInited()) ImGui::EndDisabled();

        ImGui::PopID();
        ImGui::PopID();
    }
}
