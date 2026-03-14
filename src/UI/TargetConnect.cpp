#include "../../include/UI/TargetConnect.h"

#include <filesystem>
#include <vector>
#include "fontawesome.h"

#include "interfaces/RCP_Interface.h"

#include "UI/style.h"
#include "hardware/HardwareControl.h"
#include "utils.h"
#include "UI/UIControl.h"

namespace LRI::RCI::TargetConnect {
    namespace SerialPort {
        RCP_Interface* render() {
            return nullptr;
        }
    }

    namespace {
        std::vector<std::pair<std::filesystem::path, std::string>> jsons;
        size_t selectedJson = 0;

        void refreshJsons() {
            jsons.clear();

            const auto& targets = getRoamingFolder() / "targets";
            for(const auto& file : std::filesystem::directory_iterator(targets)) {
                if(file.is_directory() || !file.path().string().ends_with(".json")) continue;
                jsons.emplace_back(file.path(), file.path().filename().string());
            }
        }
    }

    RCP_Interface* render(const Box& region) {
        if(jsons.empty()) refreshJsons();

        ImGui::SetNextWindowPos(region.tl());
        ImGui::SetNextWindowSize(region.size());
        ImGui::Begin("##targetconnect", nullptr, WFLAGS);

        ImGui::Text("Choose Target Config: ");
        ImGui::SameLine();

        if(jsons.empty()) ImGui::Text("No available configs");
        else if(ImGui::BeginCombo("##targetconfigcombo", jsons[selectedJson].second.c_str())) {
            for(size_t i = 0; i < jsons.size(); i++) {
                bool selected = i == selectedJson;
                if(ImGui::Selectable(jsons[i].second.c_str(), &selected)) selectedJson = i;
                if(selected) ImGui::SetItemDefaultFocus();
            }

            ImGui::EndCombo();
        }

        ImGui::SameLine();
        if(ImGui::Button("" ICON_FA_ROTATE_LEFT "##refreshjsons")) {
            refreshJsons();
            selectedJson = 0;
        }

        ImGui::Text("Polling Rate: ");
        ImGui::SetNextItemWidth(scale(100));
        ImGui::SameLine();
        ImGui::InputInt("##hwctrlpollingrateinput", &HWCTRL::POLLS_PER_UPDATE, 1, 5);
        if(HWCTRL::POLLS_PER_UPDATE < 1) HWCTRL::POLLS_PER_UPDATE = 1;
        ImGui::Text("Packets processed in last frame: %d", HWCTRL::PACKETS_POLLED_IN_LAST_FRAME);

        RCP_Interface* interf = nullptr;

        ImGui::NewLine();
        if(ImGui::BeginTabBar("##connecttab")) {
            if(ImGui::BeginTabItem("Serial Port", nullptr)) {
                interf = SerialPort::render();
                ImGui::EndTabItem();
            }

            if(ImGui::BeginTabItem("TCP Socket", nullptr)) ImGui::EndTabItem();
            if(ImGui::BeginTabItem("Virtual Port", nullptr)) ImGui::EndTabItem();
            ImGui::EndTabBar();
        }

        ImGui::End();

        return interf;
    }
} // namespace LRI::RCI::TargetConnect
