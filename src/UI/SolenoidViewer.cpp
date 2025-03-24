#include "UI/SolenoidViewer.h"

#include <imgui.h>
#include <utils.h>
#include <thread>
#include <chrono>

#include "RCP_Host/RCP_Host.h"

namespace LRI::RCI {
    SolenoidViewer::SolenoidViewer(const std::set<HardwareQualifier>& _sols, const bool refreshButton)
        : refreshButton(refreshButton) {
        for(const auto& sol : _sols) {
            sols[sol] = Solenoids::getInstance()->getState(sol);
        }
    }

    void SolenoidViewer::render() {
        ImDrawList* draw = ImGui::GetWindowDrawList();

        bool lockButtons = buttonTimer.timeSince() < BUTTON_DELAY;
        if(lockButtons) ImGui::BeginDisabled();

        // A button to manually refresh the states of each solenoid
        if(refreshButton && ImGui::Button("Invalidate Cache and Refresh States")) {
            Solenoids::getInstance()->refreshAll();
            buttonTimer.reset();
        }
        if(lockButtons) ImGui::EndDisabled();
        ImGui::Separator();

        // Rendering each solenoid is simple. It consists of a status square, and a button to turn the solenoid
        // on and off
        for(const auto& [id, state] : sols) {
            ImVec2 pos = ImGui::GetCursorScreenPos();
            ImU32 statusColor = !state->stale ? (state->open ? ENABLED_COLOR : DISABLED_COLOR) : STALE_COLOR;
            const char* tooltip = !state->stale ? (state->open ? "Solenoid ON" : "Solenoid OFF") : "Stale Data";
            draw->AddRectFilled(pos, pos + scale(STATUS_SQUARE_SIZE), statusColor);
            ImGui::Dummy(scale(STATUS_SQUARE_SIZE));
            if(ImGui::IsItemHovered()) ImGui::SetTooltip(tooltip);
            ImGui::SameLine();
            ImGui::Text("Solenoid %s (%d)", id.name.c_str(), id.id);

            ImGui::SameLine();

            if(lockButtons || !state->stale) ImGui::BeginDisabled();
            if(ImGui::Button((std::string(state ? "ON##" : "OFF##") + id.asString()).c_str())) {
                Solenoids::getInstance()->setSolenoidState(id, state->open ? RCP_SOLENOID_OFF : RCP_SOLENOID_ON);
                buttonTimer.reset();
            }
            if(lockButtons || !state->stale) ImGui::EndDisabled();
        }
    }
}
