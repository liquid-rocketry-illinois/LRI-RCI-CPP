#include "UI/EStop.h"

#include "imgui.h"
#include "RCP_Host/RCP_Host.h"

#include "utils.h"

namespace LRI::RCI {
    constexpr int winFlags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoResize;
    EStop* EStop::instance;
    EStop::EStop(): BaseUI() {}

    EStop* const EStop::getInstance() {
        if(instance == nullptr) instance = new EStop();
        return instance;
    }

    void EStop::render() {
        ImGui::SetNextWindowPos(scale(ImVec2(450, 275)), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(scale(ImVec2(100, 100)), ImGuiCond_FirstUseEver);

        if(ImGui::Begin("ESTOP", nullptr, winFlags)) {
            if(!RCP_isOpen()) ImGui::BeginDisabled();
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1, 0, 0, 1));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.7, 0, 0, 1));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.9, 0, 0, 1));
            if(ImGui::Button("EMERGENCY\n  STOP", ImVec2(90 * scaling_factor, 70 * scaling_factor))) {
                RCP_sendEStop();
            }

            ImGui::PopStyleColor(3);
            if(!RCP_isOpen()) ImGui::EndDisabled();
        }

        ImGui::End();
    }
}