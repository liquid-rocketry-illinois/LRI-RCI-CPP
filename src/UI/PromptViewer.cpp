#include "UI/PromptViewer.h"

#include "hardware/Prompt.h"

namespace LRI::RCI {
    PromptViewer::PromptViewer(bool standaloneWindow, const ImVec2&& startPos, const ImVec2&& startSize)
        : standaloneWindow(standaloneWindow), startPos(scale(startPos)), startSize(scale(startSize)) {
    }

    void PromptViewer::render() {
        if(standaloneWindow) {
            ImGui::SetNextWindowPos(startPos, ImGuiCond_FirstUseEver);
            ImGui::SetNextWindowSize(startSize, ImGuiCond_FirstUseEver);
            if(!ImGui::Begin("Target Prompt")) {
                ImGui::End();
                return;
            }
        }

        if(!Prompt::getInstance()->is_active_prompt()) {
            ImGui::Text("No Active Prompt");
            ImGui::End();
            return;
        }

        ImGui::TextUnformatted(Prompt::getInstance()->get_prompt().c_str());

        if(Prompt::getInstance()->getType() == RCP_PromptDataType_GONOGO) {
            bool* gng = Prompt::getInstance()->getGNGPointer();
            bool prev = *gng;
            if(prev) ImGui::BeginDisabled();
            if(ImGui::Button("NO GO")) *gng = false;
            if(prev) ImGui::EndDisabled();

            if(!prev) ImGui::BeginDisabled();
            ImGui::SameLine();
            if(ImGui::Button("GO")) *gng = true;
            if(!prev) ImGui::EndDisabled();

            ImGui::SameLine();
            ImGui::PushStyleColor(ImGuiCol_Text, prev ? ImVec4(0, 1, 0, 1) : ImVec4(1, 0, 0, 1));
            ImGui::Text(prev ? "GO" : "NO GO");
            ImGui::PopStyleColor();
        }

        else {
            ImGui::Text("Enter Value: ");
            ImGui::SameLine();
            ImGui::InputFloat("##promptfloatval", Prompt::getInstance()->getValPointer());
        }

        if(ImGui::Button("Confirm##promptconfirm")) {
            Prompt::getInstance()->submitPrompt();
        }

        if(standaloneWindow) ImGui::End();
    }
}
