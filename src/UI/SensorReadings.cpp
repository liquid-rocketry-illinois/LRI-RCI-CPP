#include <utility>

#include "UI/SensorReadings.h"
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

    SensorReadings* SensorReadings::getInstance() {
        if(instance == nullptr) instance = new SensorReadings();
        return instance;
    }

    float min3(float a, float b, float c) {
        return min(a, min(b, c));
    }

    void SensorReadings::render() {
        ImGui::SetNextWindowPos(scale(ImVec2(675, 50)), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(scale(ImVec2(700, 300)), ImGuiCond_FirstUseEver);
        if(ImGui::Begin("Sensor Readouts")) {
            ImDrawList* draw = ImGui::GetWindowDrawList();
            float xsize = ImGui::GetWindowWidth() - scale(50);
            ImVec2 plotsize = ImVec2(xsize,
                min3(xsize * (9.0f / 16.0f), scale(500), ImGui::GetWindowHeight() - scale(75)));
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

                if(qual.devclass == RCP_DEVCLASS_GPS) {
                    if(ImPlot::BeginPlot((std::string("Sensor Data##") + qual.asString() + ":latlon").c_str(),
                                         plotsize)) {
                        ImPlot::SetupAxes("Time (s)", "Latitude/Longitude (degrees)");
                        if(data.empty())
                            ImPlot::PlotLine((std::string("Latitude##") + qual.asString()).c_str(), &data[0].timestamp,
                                             &data[0].data.gpsData[0], static_cast<int>(data.size()), 0, 0,
                                             sizeof(DataPoint));

                        if(!data.empty())
                            ImPlot::PlotLine((std::string("Longitude##") + qual.asString()).c_str(), &data[0].timestamp,
                                             &data[0].data.gpsData[1], static_cast<int>(data.size()), 0, 0,
                                             sizeof(DataPoint));

                        ImPlot::EndPlot();
                    }

                    if(ImPlot::BeginPlot((std::string("Sensor Data##") + qual.asString() + ":alt").c_str(),
                                         plotsize)) {
                        ImPlot::SetupAxes("Time (s)", "Altitude (m)");
                        if(!data.empty())
                            ImPlot::PlotLine((std::string("Altitude##") + qual.asString()).c_str(), &data[0].timestamp,
                                             &data[0].data.gpsData[2], static_cast<int>(data.size()), 0, 0,
                                             sizeof(DataPoint));

                        ImPlot::EndPlot();
                    }

                    if(ImPlot::BeginPlot((std::string("Sensor Data##") + qual.asString() + ":gs").c_str(),
                                         plotsize)) {
                        ImPlot::SetupAxes("Time (s)", "Ground Speed (m/s)");
                        if(!data.empty())
                            ImPlot::PlotLine((std::string("Ground Speed##") + qual.asString()).c_str(),
                                             &data[0].timestamp,
                                             &data[0].data.gpsData[3], static_cast<int>(data.size()), 0, 0,
                                             sizeof(DataPoint));

                        ImPlot::EndPlot();
                    }
                }

                else if(ImPlot::BeginPlot((std::string("Sensor Data##") + qual.asString()).c_str(), plotsize)) {
                    ImPlot::SetNextMarkerStyle(ImPlotMarker_Circle);
                    std::string graphname;
                    switch(qual.devclass) {
                    case RCP_DEVCLASS_PRESSURE_TRANSDUCER:
                    case RCP_DEVCLASS_AM_PRESSURE:
                        ImPlot::SetupAxes("Time (s)", "Pressure (millibars)");
                        graphname = "Pressure##";
                        break;

                    case RCP_DEVCLASS_MAGNETOMETER:
                        ImPlot::SetupAxes("Time (s)", "Magnetic Strength (Gauss)");
                        break;

                    case RCP_DEVCLASS_AM_TEMPERATURE:
                        ImPlot::SetupAxes("Time (s)", "Temperature (Celcius)");
                        graphname = "Temperature##";
                        break;

                    case RCP_DEVCLASS_ACCELEROMETER:
                        ImPlot::SetupAxes("Time (s)", "Acceleration (m/s/s)");
                        break;

                    case RCP_DEVCLASS_GYROSCOPE:
                        ImPlot::SetupAxes("Time (s)", "Angular Acceleration (d/s/s)");
                        break;

                    default:
                        ImPlot::SetupAxes("Unknown Data", "Unknown Data");
                        break;
                    }
                    if(!data.empty())
                        switch(qual.devclass) {
                        case RCP_DEVCLASS_PRESSURE_TRANSDUCER:
                        case RCP_DEVCLASS_AM_PRESSURE:
                        case RCP_DEVCLASS_AM_TEMPERATURE:
                            ImPlot::PlotLine((graphname + qual.asString()).c_str(), &data[0].timestamp,
                                             &data[0].data.singleVal, static_cast<int>(data.size()),
                                             0, 0, sizeof(DataPoint));
                            break;

                        case RCP_DEVCLASS_MAGNETOMETER:
                        case RCP_DEVCLASS_ACCELEROMETER:
                        case RCP_DEVCLASS_GYROSCOPE:
                            ImPlot::PlotLine((std::string("X") + qual.asString()).c_str(), &data[0].timestamp,
                                             &data[0].data.axisData[0], static_cast<int>(data.size()), 0, 0,
                                             sizeof(DataPoint));

                            ImPlot::PlotLine((std::string("Y") + qual.asString()).c_str(), &data[0].timestamp,
                                             &data[0].data.axisData[1], static_cast<int>(data.size()), 0, 0,
                                             sizeof(DataPoint));

                            ImPlot::PlotLine((std::string("Z") + qual.asString()).c_str(), &data[0].timestamp,
                                             &data[0].data.axisData[2], static_cast<int>(data.size()), 0, 0,
                                             sizeof(DataPoint));

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
        }
    }

    void SensorReadings::receiveRCPUpdate(const SensorQualifier& qual, const DataPoint& data) {
        sensors[qual].push_back(data);
    }
}
