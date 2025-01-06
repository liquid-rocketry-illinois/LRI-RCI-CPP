#include "UI/Solenoids.h"

#include <imgui.h>
#include <iostream>
#include <utils.h>
#include <thread>
#include <chrono>

#include "RCP_Host/RCP_Host.h"

namespace LRI::RCI {
    Solenoids* Solenoids::instance;

    Solenoids* Solenoids::getInstance() {
        if(instance == nullptr) instance = new Solenoids();
        return instance;
    }

    void Solenoids::render() {
        ImGui::SetNextWindowPos(scale(ImVec2(675, 400)), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(scale(ImVec2(350, 200)), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowCollapsed(true, ImGuiCond_FirstUseEver);
        if(ImGui::Begin("Manual Solenoid Control")) {
            ImDrawList* draw = ImGui::GetWindowDrawList();

            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1, 0, 0, 1));
            ImGui::PushTextWrapPos(0);
            ImGui::Text("WARNING: This panel is for manual control only. Solenoid states may be stale.");
            ImGui::PopTextWrapPos();
            ImGui::PopStyleColor();

            ImGui::NewLine();
            time_t now;
            time(&now);
            bool lockButtons = buttonTimer.timeSince() < BUTTON_DELAY;
            if(lockButtons) ImGui::BeginDisabled();
            if(ImGui::Button("Invalidate Cache and Refresh States")) {
                for(const auto [id, state] : sols) {
                    solUpdated[id] = false;
                    RCP_requestSolenoidRead(id);
                }
                buttonTimer.reset();
            }
            if(lockButtons) ImGui::EndDisabled();
            ImGui::Separator();

            for(const auto& [id, state] : sols) {
                ImVec2 pos = ImGui::GetCursorScreenPos();
                ImU32 statusColor = solUpdated[id] ? (state ? ENABLED_COLOR : DISABLED_COLOR) : STALE_COLOR;
                const char* tooltip = solUpdated[id] ? (state ? "Solenoid ON" : "Solenoid OFF") : "Stale Data";
                draw->AddRectFilled(pos, pos + scale(STATUS_SQUARE_SIZE), statusColor);
                ImGui::Dummy(scale(STATUS_SQUARE_SIZE));
                if(ImGui::IsItemHovered()) ImGui::SetTooltip(tooltip);
                ImGui::SameLine();
                ImGui::Text("Solenoid %s (%d)", solname[id].c_str(), id);

                ImGui::SameLine();

                if(lockButtons || !solUpdated[id]) ImGui::BeginDisabled();
                if(ImGui::Button((std::string(state ? "ON##" : "OFF##") + std::to_string(id)).c_str())) {
                    RCP_sendSolenoidWrite(id, state ? RCP_SOLENOID_OFF : RCP_SOLENOID_ON);
                    buttonTimer.reset();
                    sols[id] = !sols[id];
                }
                if(lockButtons || !solUpdated[id]) ImGui::EndDisabled();
            }
        }

        ImGui::End();
    }

    void Solenoids::setHardwareConfig(const std::map<uint8_t, std::string>& solIds) {
        sols.clear();
        solUpdated.clear();

        for(const auto& [id, name] : solIds) {
            sols[id] = false;
            solUpdated[id] = false;
            solname[id] = name.empty() ? std::to_string(id) : name;
            RCP_requestSolenoidRead(id);
        }
        buttonTimer.reset();
    }

    void Solenoids::receiveRCPUpdate(const RCP_SolenoidData& data) {
        sols[data.ID] = data.state == RCP_SOLENOID_ON;
        solUpdated[data.ID] = true;
    }
}
