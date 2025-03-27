#include <utility>
#include <fstream>
#include <filesystem>
#include <chrono>
#include <ranges>

#include "UI/SensorViewer.h"
#include <implot.h>

namespace LRI::RCI {
    int SensorViewer::classnum = 0;

    // Helper
    float min3(float a, float b, float c) {
        return min(a, min(b, c));
    }

    void SensorViewer::renderLatestReadings(const HardwareQualifier& qual, const Sensors::DataPoint& data) {
        switch(qual.devclass) {
        case RCP_DEVCLASS_AM_PRESSURE:
            ImGui::Text("Pressure: %.3f mbar", data.data[0]);
            break;

        case RCP_DEVCLASS_PRESSURE_TRANSDUCER:
            ImGui::Text("Pressure: %.3f psi", data.data[0]);
            break;

        case RCP_DEVCLASS_AM_TEMPERATURE:
            ImGui::Text("Temperature: %.3f C", data.data[0]);
            break;

        case RCP_DEVCLASS_RELATIVE_HYGROMETER:
            ImGui::Text("Humidity: %.3f %%", data.data[0]);
            break;

        case RCP_DEVCLASS_LOAD_CELL:
            ImGui::Text("Mass: %.3f kg", data.data[0]);
            break;

        case RCP_DEVCLASS_POWERMON:
            ImGui::Text("Voltage: %.3f V | Power: %.3f W", data.data[0], data.data[1]);
            break;

        case RCP_DEVCLASS_ACCELEROMETER:
            ImGui::Text("X: %.3f m/s/s | Y: %.3f m/s/s | Z: %.3f m/s/s", data.data[0], data.data[1], data.data[2]);
            break;

        case RCP_DEVCLASS_GYROSCOPE:
            ImGui::Text("X: %.3f d/s/s | Y: %.3f d/s/s | Z: %.3f d/s/s", data.data[0], data.data[1], data.data[2]);
            break;

        case RCP_DEVCLASS_MAGNETOMETER:
            ImGui::Text("X: %.3f G | Y: %.3f G | Z: %.3f G", data.data[0], data.data[1], data.data[2]);
            break;

        case RCP_DEVCLASS_GPS:
            ImGui::Text("Latitude: %.3f d | Longitude: %.3f d | Altitude: %.3f m | Ground Speed: %.3f m/s",
                        data.data[0], data.data[1], data.data[2], data.data[3]);
            break;

        default:
            ImGui::Text("Unrecognized sensor");
            break;
        }
    }

    SensorViewer::SensorViewer(const std::map<HardwareQualifier, bool>& quals)
        : sensorchild("##sensorchild" + std::to_string(classnum++)) {
        for(const auto& [qual, abr] : quals) {
            sensors[qual] = Sensors::getInstance()->getState(qual);
            abridged[qual] = abr;
        }
    }

    void SensorViewer::render() {
        ImDrawList* draw = ImGui::GetWindowDrawList();
        const float xsize = ImGui::GetWindowWidth() - scale(50);
        const ImVec2 plotsize = ImVec2(xsize, min3(xsize * (9.0f / 16.0f), scale(500),
                                                   ImGui::GetWindowHeight() - scale(75)));

        if(!ImGui::BeginChild(sensorchild.c_str(), ImGui::GetWindowSize() - scale(ImVec2(0, 40)))) {
            ImGui::EndChild();
            return;
        }

        int num = 0;
        for(const auto& [qual, data] // vv-- Filter for abridged sensors
            : sensors | std::views::filter([&](const auto& a) { return abridged[a.first]; })) {
            renderLatestReadings(qual, data->empty() ? empty : data[data->size() - 1]);
            if(num++ % 4 != 0) {
                ImGui::SameLine();
                ImGui::Text(" | ");
                ImGui::SameLine();
            }
        }

        for(const auto& [qual, data] // Filter for non-abridged sensors
            : sensors | std::views::filter([&](const auto& a) { return !abridged[a.first]; })) {
            if(!ImGui::TreeNode((qual.name + sensorchild + qual.asString()).c_str())) continue;

            ImGui::Text("Sensor Status: ");
            ImGui::SameLine();
            ImVec2 pos = ImGui::GetCursorScreenPos();
            draw->AddRectFilled(pos, pos + scale(STATUS_SQUARE_SIZE),
                                data->empty() ? STALE_COLOR : ENABLED_COLOR);
            ImGui::Dummy(scale(STATUS_SQUARE_SIZE));
            if(ImGui::IsItemHovered()) ImGui::SetTooltip(data->empty() ? "No data received" : "Receiving data");

            ImGui::SameLine();
            ImGui::Text(" | ");
            ImGui::SameLine();
            if(ImGui::Button(("Write to CSV" + sensorchild + qual.asString() + sensorchild).c_str()))
                Sensors::getInstance()->writeCSV(qual);

            renderGraphs(qual, data, plotsize);

            ImGui::Separator();
            ImGui::TreePop();
        }

        ImGui::EndChild();
    }

    void SensorViewer::renderGraphs(const HardwareQualifier& qual, const std::vector<Sensors::DataPoint>* data,
                                    const ImVec2& plotsize) const {
        for(const auto& graph : GRAPHINFO.at(qual.devclass)) {
            if(!ImPlot::BeginPlot((std::string(graph.name) + sensorchild + qual.asString()).c_str(), plotsize))
                continue;

            ImPlot::SetupAxes("Time (s)", graph.axis, ImPlotAxisFlags_AutoFit, ImPlotAxisFlags_AutoFit);
            if(data->empty()) {
                ImPlot::EndPlot();
                continue;
            }

            for(const auto& line : graph.lines) {
                ImPlot::PlotLine((line.name + sensorchild + qual.asString()).c_str(),
                                 &data->at(0).timestamp,
                                 (data->at(0).data + line.datanum), static_cast<int>(data->size()),
                                 0, 0, sizeof(Sensors::DataPoint));
            }

            ImPlot::EndPlot();
        }
    }
}
