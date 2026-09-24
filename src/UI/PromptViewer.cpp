#include "UI/PromptViewer.h"

// #include "utils.h"
#include "hardware/hwctrl.h"

// Module for viewing latest prompt state
namespace LRI::RCI {
    void PromptViewer::render() {
        ImGui::PushID("PromptViewer");
        ImGui::PushID(classid);

        bool disable = !hwctrl::isTargetReady();
        if(disable) ImGui::BeginDisabled();

        // If there is no active prompt display that message and exit early
        RCP_PromptDataType ptype = hwctrl::promptType();
        if(ptype == RCP_PromptDataType_RESET) {
            ImGui::Text("No Active Prompt");
            if(disable) ImGui::EndDisabled();
            ImGui::PopID();
            ImGui::PopID();
            return;
        }

        ImGui::PushTextWrapPos();
        ImGui::TextUnformatted(hwctrl::promptString().c_str());
        ImGui::PopTextWrapPos();

        // Draw different things based on the datatype of the prompt

        // If the prompt type is a GO/NO GO verification:
        if(ptype == RCP_PromptDataType_GONOGO) {
            // If the current state is GO:
            bool prev = bprompt;
            if(!prev) ImGui::BeginDisabled();
            if(ImGui::Button("NO GO")) bprompt = true;
            if(!prev) ImGui::EndDisabled();

            // If the current state is NO GO
            if(prev) ImGui::BeginDisabled();
            ImGui::SameLine();
            if(ImGui::Button("GO")) bprompt = RCP_GONOGO_GO;
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
            ImGui::InputFloat("##promptfloatval", &fprompt);
        }

        // Lock the confirm if the test isnt running
        bool lock = hwctrl::getTestState() != RCP_TEST_RUNNING;
        if(lock) ImGui::BeginDisabled();
        if(ImGui::Button("Confirm")) {
            if(ptype == RCP_PromptDataType_Float) hwctrl::promptRespond(fprompt);
            else hwctrl::promptRespond(bprompt);
        }
        if(lock) ImGui::EndDisabled();

        if(disable) ImGui::EndDisabled();
        ImGui::PopID();
        ImGui::PopID();
    }
} // namespace LRI::RCI
