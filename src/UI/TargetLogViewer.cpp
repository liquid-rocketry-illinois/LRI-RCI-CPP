#include "UI/TargetLogViewer.h"
#include "UI/gutils.h"

#include "hardware/EventLog.h"
#include "hardware/hwctrl.h"

// Module for viewing latest debug output
namespace LRI::RCI {
    void TargetLogViewer::render() {
        ImGui::PushID("RawViewer");
        ImGui::PushID(classid);

        ImGui::Checkbox("Autoscroll", &autoscroll);
        // An imgui child to contain the text
        if(ImGui::BeginChild("##serialchild", {ImGui::GetWindowWidth(), 175_sc})) {
            if(autoscroll) ImGui::SetScrollY(ImGui::GetScrollMaxY());
            const auto& [times, logs] = hwctrl::getELog()->getLogs();
            for(size_t i = 0; i < times->size(); i++) {
                std::string line = std::format("[{:%H-%M-%OS}] {}", times->at(i), logs->at(i));
                ImGui::TextWrapped("%s", line.c_str());
            }
        }

        ImGui::EndChild();
        ImGui::Separator();

        ImGui::PopID();
        ImGui::PopID();
    }
} // namespace LRI::RCI
