#include "UI/Solenoids.h"

#include <imgui.h>
#include <iostream>
#include <string>
#include <utils.h>

#include "RCP_Host/RCP_Host.h"

namespace LRI::RCI {
    Solenoids* Solenoids::instance;

    Solenoids* const Solenoids::getInstance() {
        if(instance == nullptr) instance = new Solenoids();
        return instance;
    }

    void Solenoids::render() {
        ImGui::SetNextWindowPos(scale(ImVec2(50, 300)), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(scale(ImVec2(
                                           buttonLeftMargin + ((buttonSize + buttonLeftMargin) * 4),
                                           ((sols.size() / solsPerRow) * (buttonSize + buttonTopMargin)) +
                                           buttonTopMargin +
                                           buttonExtraMargin)), ImGuiCond_FirstUseEver);

        if(ImGui::Begin("Manual Solenoid Control")) {
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1, 0, 0, 1));
            ImGui::Text("WARNING: This panel is for manual\ncontrol only. Solenoid states may be stale");
            ImGui::PopStyleColor();

            ImGui::NewLine();
            time_t now;
            time(&now);
            bool lockButtons = buttonTimer.timeSince() < buttonDelay;
            if(lockButtons) ImGui::BeginDisabled();
            if(ImGui::Button("Invalidate Cache and Refresh States")) {
                for(const auto [id, state] : sols) {
                    solUpdated[id] = false;
                    RCP_requestSolenoidRead(id);
                }
                buttonTimer.reset();
            }

            int pos = 0;
            for(const auto [id, state] : sols) {
                ImGui::SetCursorPos(scale(ImVec2(
                                              buttonLeftMargin + ((pos % solsPerRow) * (buttonLeftMargin + buttonSize)),
                                              buttonExtraMargin + buttonTopMargin + ((pos / solsPerRow) * (
                                                  buttonTopMargin + buttonSize))
                                          )));

                if(!solUpdated[id]) {
                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1, 1, 0, 1));
                    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1, 1, 0, 1));
                    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(1, 1, 0, 1));
                }

                else if(state) {
                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 1, 0, 1));
                    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0, 0.9, 0, 1));
                    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0, 0.7, 0, 1));
                }

                else {
                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1, 0, 0, 1));
                    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.9, 0, 0, 1));
                    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.7, 0, 0, 1));
                }

                if(!solUpdated[id]) ImGui::BeginDisabled();
                if(ImGui::Button((std::string("ID: ") + std::to_string(id) + "##solbutton").c_str(),
                                 ImVec2(buttonSize * scaling_factor, buttonSize * scaling_factor))) {
                    RCP_SolenoidState newstate = state ? RCP_SOLENOID_OFF : RCP_SOLENOID_ON;
                    RCP_sendSolenoidWrite(id, newstate);
                    buttonTimer.reset();
                    sols[id] = !sols[id];
                }
                if(!solUpdated[id]) ImGui::EndDisabled();

                ImGui::PopStyleColor(3);
                pos++;
            }

            if(lockButtons) ImGui::EndDisabled();
        }

        ImGui::End();
    }

    void Solenoids::setHardwareConfig(const std::set<uint8_t>& solIds) {
        sols.clear();
        solUpdated.clear();

        for(const auto& i : solIds) {
            sols[i] = false;
            solUpdated[i] = false;
            RCP_requestSolenoidRead(i);
        }
        buttonTimer.reset();
    }

    void Solenoids::receiveRCPUpdate(const uint8_t id, RCP_SolenoidState_t state) {
        sols[id] = state == RCP_SOLENOID_ON;
        solUpdated[id] = true;
    }
}
