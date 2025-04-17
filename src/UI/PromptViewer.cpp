#include "UI/PromptViewer.h"

#include "hardware/Prompt.h"
#include "hardware/TestState.h"
#include "utils.h"

namespace LRI::RCI {
    int PromptViewer::CLASSID = 0;

    PromptViewer::PromptViewer() :
        classid(CLASSID++) {}

    void PromptViewer::render() {
        ImGui::PushID("PromptViewer");
        ImGui::PushID(classid);

        if(!TestState::getInited()) ImGui::BeginDisabled();

        if(!Prompt::getInstance()->is_active_prompt()) {
            ImGui::Text("No Active Prompt");
            if(!TestState::getInited()) ImGui::EndDisabled();
            ImGui::PopID();
            ImGui::PopID();
            return;
        }

        ImGui::PushTextWrapPos();
        ImGui::TextUnformatted(Prompt::getInstance()->get_prompt().c_str());
        ImGui::PopTextWrapPos();

        if(Prompt::getInstance()->getType() == RCP_PromptDataType_GONOGO) {
            RCP_GONOGO* gng = Prompt::getInstance()->getGNGPointer();
            bool prev = *gng;
            if(!prev) ImGui::BeginDisabled();
            if(ImGui::Button("NO GO")) *gng = RCP_GONOGO_NOGO;
            if(!prev) ImGui::EndDisabled();

            if(prev) ImGui::BeginDisabled();
            ImGui::SameLine();
            if(ImGui::Button("GO")) *gng = RCP_GONOGO_GO;
            if(prev) ImGui::EndDisabled();

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

        bool lock = TestState::getInstance()->getState() != RCP_TEST_RUNNING;
        if(lock) ImGui::BeginDisabled();
        if(ImGui::Button("Confirm")) {
            Prompt::getInstance()->submitPrompt();
        }
        if(lock) ImGui::EndDisabled();

        if(!TestState::getInited()) ImGui::EndDisabled();
        ImGui::PopID();
        ImGui::PopID();
    }
}
