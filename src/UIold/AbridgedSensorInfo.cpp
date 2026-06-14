#include "UI/AbridgedSensorViewer.h"

#include <format>

#include "hardware/HardwareControl.h"

namespace LRI::RCI {
    AbridgedSensorViewer::AbridgedSensorViewer(const std::vector<std::vector<HardwareChannel>>& sensors) :
        sensors(sensors) {
        for(const auto& sensorset : sensors) {
            for(const auto& qual : sensorset) {
                if(data.contains(qual)) continue;
                const auto* sense = Sensors::getState(qual);
                if(sense == nullptr) {
                    HWCTRL::addError(
                        {HWCTRL::ErrorType::HWNE_HOST, "Qualifier not found in sensors list: " + qual.asString()});
                    continue;
                }

                data[qual] = sense;
            }
        }
    }

    void AbridgedSensorViewer::render() {
        if(sensors.empty()) {
            ImGui::Text("Nothing to show!");
            return;
        }

        ImGui::PushID("AbridgedSensorViewer");
        ImGui::PushID(classid);

        // A lot of this math is just for text wrapping, so that numbers dont get wrapped in the middle
        const float width = ImGui::GetWindowWidth();
        const float spacerWidth = ImGui::CalcTextSize(" | ").x;

        for(const auto& senselist : sensors) {
            float currentLineWidth = 0;

            for(const auto& qual : senselist) {
                auto datavec = data[qual];
                std::string str = Sensors::renderLatestReadingsString(
                    qual, datavec->empty() ? Sensors::empty : datavec->at(datavec->size() - 1));
                float size = ImGui::CalcTextSize(str.c_str()).x * 1.075f;

                if(currentLineWidth + size > width || currentLineWidth == 0) {
                    ImGui::TextUnformatted(str.c_str());
                    currentLineWidth = size;
                }

                else {
                    ImGui::SameLine();
                    ImGui::Text(" | %s", str.c_str());
                    currentLineWidth += size + spacerWidth;
                }
            }
        }

        ImGui::PopID();
        ImGui::PopID();
    }
} // namespace LRI::RCI
