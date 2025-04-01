#include "UI/RawViewer.h"
#include "utils.h"
#include "hardware/RawData.h"

namespace LRI::RCI {
    int RawViewer::CLASSID = 0;

    RawViewer::RawViewer() :
        classid(CLASSID++) {}

    void RawViewer::render() {
        ImGui::PushID("RawViewer");
        ImGui::PushID(classid);

        if(ImGui::Button("Clear")) {
            RawData::getInstance()->reset();
        }

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

        ImGui::PopID();
        ImGui::PopID();
    }
}
