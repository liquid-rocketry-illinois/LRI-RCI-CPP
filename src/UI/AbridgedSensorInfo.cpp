#include "UI/AbridgedSensorViewer.h"

#include "hardware/hwctrl.h"
#include "UI/gutils.h"

namespace LRI::RCI {
    AbridgedSensorViewer::AbridgedSensorViewer(const std::vector<std::vector<HardwareChannel>>& sensors) :
        sensors(sensors) {
        for(const auto& sensorset : sensors) {
            for(const auto& qual : sensorset) {
                if(data.contains(qual)) continue;
                data[qual] = hwctrl::getELog()->getChannelFloatData(qual);
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
                std::string str = renderLatestReadingsString(qual, datavec.values->empty() ? 0 : datavec.values->back());
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
