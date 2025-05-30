#include "UI/PromptViewer.h"

#include "hardware/Prompt.h"
#include "hardware/TestState.h"
#include "utils.h"

// Module for viewing latest prompt state
namespace LRI::RCI {
    void PromptViewer::render() {
        ImGui::PushID("PromptViewer");
        ImGui::PushID(classid);

        if(!TestState::getInited()) ImGui::BeginDisabled();

        // If there is no active prompt display that message and exit early
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

        // Draw different things based on the datatype of the prompt

        // If the prompt type is a GO/NO GO verification:
        if(Prompt::getInstance()->getType() == RCP_PromptDataType_GONOGO) {
            // Get the pointer so if the user changes the state we can update the singleton
            RCP_GONOGO* gng = Prompt::getInstance()->getGNGPointer();
            bool prev = *gng;

            // If the current state is GO:
            if(!prev) ImGui::BeginDisabled();
            if(ImGui::Button("NO GO")) *gng = RCP_GONOGO_NOGO;
            if(!prev) ImGui::EndDisabled();

            // If the current state is NO GO
            if(prev) ImGui::BeginDisabled();
            ImGui::SameLine();
            if(ImGui::Button("GO")) *gng = RCP_GONOGO_GO;
            if(prev) ImGui::EndDisabled();

            // Show the currently selected option
            ImGui::SameLine();
            ImGui::PushStyleColor(ImGuiCol_Text, prev ? ImVec4(0, 1, 0, 1) : ImVec4(1, 0, 0, 1));
            ImGui::Text(prev ? "GO" : "NO GO");
            ImGui::PopStyleColor();
        }

        // If the current prompt type is float:
        else {
            ImGui::Text("Enter Value: ");
            ImGui::SameLine();
            ImGui::InputFloat("##promptfloatval", Prompt::getInstance()->getValPointer());
        }

        // Lock the confirm if the test isnt running
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
} // namespace LRI::RCI
