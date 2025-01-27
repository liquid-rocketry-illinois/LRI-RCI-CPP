#include "UI/EStop.h"

#include "imgui.h"
#include "RCP_Host/RCP_Host.h"

#include "utils.h"

namespace LRI::RCI {

    // Very simple class
    constexpr int winFlags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoResize;
    EStop* EStop::instance;

    EStop::EStop() : BaseUI() {}

    EStop* EStop::getInstance() {
        if(instance == nullptr) instance = new EStop();
        return instance;
    }

    void EStop::render() {
        ImGui::SetNextWindowPos(scale(ImVec2(450, 300)), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(scale(ImVec2(100, 100)), ImGuiCond_FirstUseEver);

        if(ImGui::Begin("ESTOP", nullptr, winFlags)) {
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1, 0, 0, 1));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.7, 0, 0, 1));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.9, 0, 0, 1));

            // If button pushed, send E-STOP packet
            if(ImGui::Button("EMERGENCY\n  STOP", ImVec2(scale(90), scale(70)))) {
                RCP_sendEStop();
            }

            ImGui::PopStyleColor(3);
        }

        ImGui::End();
    }
}