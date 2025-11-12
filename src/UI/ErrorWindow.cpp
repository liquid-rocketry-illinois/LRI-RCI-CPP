#include <format>

#include "UI/ErrorWindow.h"

#include "hardware/HardwareControl.h"

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

        for(const auto& error : HWCTRL::getErrors()) {
            std::stringstream str;
            str << std::format("[{:%H-%M-%OS}] ", error.time);

            switch(error.type) {
            case HWCTRL::ErrorType::GENERAL_RCP:
                ImGui::PushStyleColor(ImGuiCol_Text, STALE_COLOR);
                str << "[WARNING] ";
                break;

            case HWCTRL::ErrorType::HWNE_HOST:
            case HWCTRL::ErrorType::HWNE_TARGET:
                ImGui::PushStyleColor(ImGuiCol_Text, ENABLED_COLOR);
                str << "[HWQUAL ERROR] ";
                break;

            case HWCTRL::ErrorType::RCP_STREAM:
                ImGui::PushStyleColor(ImGuiCol_Text, DISABLED_COLOR);
                str << "[RCP ERROR] ";
                break;
            }

            str << error.what;
            ImGui::TextWrapped("%s", str.str().c_str());
            ImGui::PopStyleColor();
        }
        ImGui::EndChild();

        ImGui::PopID();
        ImGui::PopID();
    }
}