#include "UI/RawViewer.h"
#include "hardware/TargetLog.h"
#include "hardware/TestState.h"
#include "utils.h"

// Module for viewing latest debug output
namespace LRI::RCI {
    void RawViewer::render() {
        ImGui::PushID("RawViewer");
        ImGui::PushID(classid);

        // Provide an easy way to clear the output
        if(ImGui::Button("Clear")) {
            RawData::reset();
        }

        if(!TestState::getInited()) ImGui::BeginDisabled();
        ImGui::Separator();

        // An imgui child to contain the text
        if(ImGui::BeginChild("##serialchild", {ImGui::GetWindowWidth(), scale(175)})) {
            // Displaying the text just by rendering the string stream
            ImGui::PushTextWrapPos();
            ImGui::TextUnformatted(RawData::getData().c_str());
            ImGui::PopTextWrapPos();
        }

        ImGui::EndChild();
        ImGui::Separator();

        if(!TestState::getInited()) ImGui::EndDisabled();
        ImGui::PopID();
        ImGui::PopID();
    }
} // namespace LRI::RCI
