#include "UI/EStopViewer.h"

#include "imgui.h"

#include "utils.h"
#include "hardware/EStop.h"

namespace LRI::RCI {
    EStopViewer::EStopViewer(const ImVec2&& size) : size(size) {
    }

    void EStopViewer::render() {
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1, 0, 0, 1));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.7, 0, 0, 1));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.9, 0, 0, 1));

        // If button pushed, send E-STOP packet
        if(ImGui::Button("EMERGENCY\n  STOP", scale(size))) {
            EStop::getInstance()->ESTOP();
        }

        ImGui::PopStyleColor(3);
    }
}
