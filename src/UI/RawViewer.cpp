#include "UI/RawViewer.h"
#include "hardware/RawData.h"

namespace LRI::RCI {
    RawViewer::RawViewer(const ImVec2&& size) : size(scale(size)) {
    }

    void RawViewer::render() {
        if(ImGui::Button("Clear##serialclear")) {
            RawData::getInstance()->reset();
        }

        ImGui::Separator();

        // An imgui child to contain the text
        if(ImGui::BeginChild("##serialchild", size)) {
            // Displaying the text just by rendering the string stream
            ImGui::PushTextWrapPos();
            ImGui::TextUnformatted(RawData::getInstance()->getData().str().c_str());
            ImGui::PopTextWrapPos();
        }

        ImGui::EndChild();
        ImGui::Separator();
    }
}
