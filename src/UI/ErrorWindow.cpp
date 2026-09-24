#include <format>

#include "UI/ErrorWindow.h"
#include "hardware/EventLog.h"
#include "hardware/hwctrl.h"

namespace LRI::RCI {
    void ErrorWindow::render() {
        ImGui::PushID("ErrorWindow");
        ImGui::PushID(classid);

        ImGui::Text("Error Output:");
        if(!ImGui::BeginChild("##errorframe")) {
            ImGui::EndChild();
            ImGui::PopID();
            ImGui::PopID();
            return;
        }

        for(const auto& error : *hwctrl::getELog()->getErrors()) {
            std::string errstr = std::format("[{:%H-%M-%OS}] [{}] {}", error.htime,
                                             error.level == Error::LWARNING ? "WARNING" : "ERROR", error.error);

            if(error.level == Error::LWARNING) ImGui::PushStyleColor(ImGuiCol_Text, STALE_COLOR);
            else ImGui::PushStyleColor(ImGuiCol_Text, DISABLED_COLOR);
            ImGui::TextWrapped("%s", errstr.c_str());
            ImGui::PopStyleColor();
        }
        ImGui::EndChild();

        ImGui::PopID();
        ImGui::PopID();
    }
} // namespace LRI::RCI
