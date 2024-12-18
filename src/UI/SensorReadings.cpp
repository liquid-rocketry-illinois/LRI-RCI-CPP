#include <utility>

#include "UI/SensorReadings.h"

#include <chrono>
#include <implot.h>

namespace LRI::RCI {
    bool SensorQualifier::operator<(SensorQualifier const& rhf) const {
        if(devclass == rhf.devclass) return id < rhf.id;
        return devclass < rhf.devclass;
    }

    std::string SensorQualifier::asString() const {
        return std::to_string(devclass) + ":" + std::to_string(id) + ":" + name;
    }

    SensorReadings* SensorReadings::instance;
    const std::map<RCP_DeviceClass_t, std::string> SensorReadings::DEVCLASS_NAMES = {};

    SensorReadings* const SensorReadings::getInstance() {
        if(instance == nullptr) instance = new SensorReadings();
        return instance;
    }

    void SensorReadings::render() {
        ImGui::SetNextWindowPos(scale(ImVec2(675, 50)), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(scale(ImVec2(700, 300)), ImGuiCond_FirstUseEver);
        if(ImGui::Begin("Sensor Readouts")) {
            ImDrawList* draw = ImGui::GetWindowDrawList();
            for(const auto& [qual, data] : sensors) {
                if(!ImGui::TreeNode(
                    (qual.name + "##" + qual.asString()).c_str()))
                    continue;
                ImGui::Text("Sensor Status: ");
                ImGui::SameLine();
                ImVec2 pos = ImGui::GetCursorScreenPos();
                draw->AddRectFilled(pos, pos + scale(STATUS_SQUARE_SIZE), data.empty() ? STALE_COLOR : ENABLED_COLOR);
                ImGui::Dummy(scale(STATUS_SQUARE_SIZE));
                if(ImGui::IsItemHovered()) ImGui::SetTooltip(data.empty() ? "No data received" : "Receiving data");

                if(ImPlot::BeginPlot((std::string("Sensor Data##") + qual.asString()).c_str(),
                                     ImVec2(ImGui::GetWindowWidth() - scale(50), scale(200)))) {
                    switch(qual.devclass) {
                    case RCP_DEVCLASS_PRESSURE_TRANSDUCER:
                        ImPlot::SetupAxes("Time (ms)", "Pressure (millibars)");
                        if(data.empty()) break;
                        ImPlot::PlotLine((std::string("Pressure##") + qual.asString()).c_str(), &data[0].timestamp,
                                         &data[0].data.singleVal, static_cast<int>(data.size()),
                                         0, 0, sizeof(DataPoint));
                        break;

                    default:
                        break;
                    }
                    ImPlot::EndPlot();
                }

                ImGui::Separator();
                ImGui::TreePop();
            }
        }

        ImGui::End();
    }

    void SensorReadings::setHardwareConfig(const std::set<SensorQualifier>& sensids) {
        sensors.clear();

        for(const auto& qual : sensids) {
            sensors[qual] = std::vector<DataPoint>();
            sensors[qual].reserve(DATA_VECTOR_INITIAL_SIZE);

            int point = 1;
            for(int i = 0; i < 50; i++) {
                DataPoint d{.timestamp = static_cast<double>(point)};
                point += 1;
                d.data.singleVal = point % 2;
                sensors[qual].push_back(d);
            }

            int j = 3;
        }
    }

    void SensorReadings::receiveRCPUpdate(const SensorQualifier& qual, const DataPoint& data) {
        sensors[qual].push_back(data);
    }
}
