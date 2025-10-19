#include "UI/ErrorWindow.h"

#include "imgui.h"

namespace LRI::RCI {
    void ErrorWindow::render() {
        ImGui::PushID("ErrorWindow");
        ImGui::PushID(classid);

        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1, 0, 0, 1));
        ImGui::Text("ERROR ENCOUNTERED!");
        ImGui::PopStyleColor();

        ImGui::TextUnformatted(errorMessage.c_str());

        ImGui::PopID();
        ImGui::PopID();
    }
}