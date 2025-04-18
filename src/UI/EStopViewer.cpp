#include "UI/EStopViewer.h"

#include "imgui.h"

#include "hardware/EStop.h"
#include "hardware/TestState.h"

// Module for viewing ESTOP state
namespace LRI::RCI {
    int EStopViewer::CLASSID = 0;

    EStopViewer::EStopViewer() :
        classid(CLASSID++) {}

    void EStopViewer::render() {
        ImGui::PushID("EStopViewer");
        ImGui::PushID(classid);

        if(!TestState::getInited()) ImGui::BeginDisabled();

        // Draw a big button that takes up all available room, and its red
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1, 0, 0, 1));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.7, 0, 0, 1));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.9, 0, 0, 1));

        // If button pushed, send E-STOP packet
        if(ImGui::Button("EMERGENCY STOP", {ImGui::GetWindowWidth(), ImGui::GetWindowWidth()})) {
            EStop::getInstance()->ESTOP();
        }

        ImGui::PopStyleColor(3);

        if(!TestState::getInited()) ImGui::EndDisabled();

        ImGui::PopID();
        ImGui::PopID();
    }
}
