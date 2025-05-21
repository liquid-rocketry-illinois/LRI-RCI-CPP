#include "UI/RawViewer.h"
#include "utils.h"
#include "hardware/RawData.h"
#include "hardware/TestState.h"

// Module for viewing latest debug output
namespace LRI::RCI {
    void RawViewer::render() {
        ImGui::PushID("RawViewer");
        ImGui::PushID(classid);

        // Provide an easy way to clear the output
        if(ImGui::Button("Clear")) {
            RawData::getInstance()->reset();
        }

        if(!TestState::getInited()) ImGui::BeginDisabled();
        ImGui::Separator();

        // An imgui child to contain the text
        if(ImGui::BeginChild("##serialchild", {ImGui::GetWindowWidth(), scale(175)})) {
            // Displaying the text just by rendering the string stream
            ImGui::PushTextWrapPos();
            ImGui::TextUnformatted(RawData::getInstance()->getData().str().c_str());
            ImGui::PopTextWrapPos();
        }

        ImGui::EndChild();
        ImGui::Separator();

        if(!TestState::getInited()) ImGui::EndDisabled();
        ImGui::PopID();
        ImGui::PopID();
    }
}
