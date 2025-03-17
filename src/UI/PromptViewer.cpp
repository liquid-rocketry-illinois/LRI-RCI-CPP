#include "UI/PromptViewer.h"

namespace LRI::RCI {
    PromptViewer* PromptViewer::instance;

    PromptViewer* PromptViewer::getInstance() {
        if(instance == nullptr) instance = new PromptViewer();
        return instance;
    }

    void PromptViewer::render() {
        ImGui::SetNextWindowPos(scale(ImVec2(1000, 400)), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(scale(ImVec2(400, 150)), ImGuiCond_FirstUseEver);
        if(ImGui::Begin("Target Prompts")) {
            ImGui::TextUnformatted(prompt.c_str());

            if(type == RCP_PromptDataType_GONOGO) {
                bool gng = gonogo;
                if(gng) ImGui::BeginDisabled();
                if(ImGui::Button("NO GO")) gonogo = false;
                if(gng) ImGui::EndDisabled();

                if(!gng) ImGui::BeginDisabled();
                ImGui::SameLine();
                if(ImGui::Button("GO")) gonogo = true;
                if(!gng) ImGui::EndDisabled();

                ImGui::SameLine();
                ImGui::PushStyleColor(ImGuiCol_Text, gng ? ImVec4(0, 1, 0, 1) : ImVec4(1, 0, 0, 1));
                ImGui::Text(gng ? "GO" : "NO GO");
                ImGui::PopStyleColor();
            }

            else {
                ImGui::Text("Enter Value: ");
                ImGui::SameLine();
                ImGui::InputFloat("##promptfloatval", &value);
            }

            if(ImGui::Button("Confirm##promptconfirm")) {
                if(type == RCP_PromptDataType_GONOGO) RCP_promptRespondGONOGO(gonogo ? RCP_GONOGO_GO : RCP_GONOGO_NOGO);
                else RCP_promptRespondFloat(value);
                gonogo = false;
                value = 0;
                hideWindow();
            }
        }

        ImGui::End();
    }

    void PromptViewer::setPrompt(const RCP_PromptInputRequest& req) {
        prompt = std::string(req.prompt);
        type = req.type;
        showWindow();
    }
}