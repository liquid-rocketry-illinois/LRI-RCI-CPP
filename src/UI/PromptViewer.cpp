#include "UI/PromptViewer.h"

#include "hardware/Prompt.h"
#include "utils.h"

namespace LRI::RCI {
    int PromptViewer::CLASSID = 0;

    PromptViewer::PromptViewer()
        : classid(CLASSID++) {
    }

    void PromptViewer::render() {
        ImGui::PushID("PromptViewer");
        ImGui::PushID(classid);

        if(!Prompt::getInstance()->is_active_prompt()) {
            ImGui::Text("No Active Prompt");
            ImGui::PopID();
            ImGui::PopID();
            return;
        }

        ImGui::PushTextWrapPos();
        ImGui::TextUnformatted(Prompt::getInstance()->get_prompt().c_str());
        ImGui::PopTextWrapPos();

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

        if(ImGui::Button("Confirm")) {
            Prompt::getInstance()->submitPrompt();
        }

        ImGui::PopID();
        ImGui::PopID();
    }
}
