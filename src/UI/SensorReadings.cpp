#include <utility>
#include <fstream>
#include <filesystem>
#include <chrono>
#include <format>

#include "UI/SensorReadings.h"
#include <implot.h>

namespace LRI::RCI {
    bool SensorQualifier::operator<(SensorQualifier const& rhf) const {
        if(devclass == rhf.devclass) return id < rhf.id;
        return devclass < rhf.devclass;
    }

    std::string SensorQualifier::asString() const {
        return std::to_string(devclass) + "-" + std::to_string(id) + "-" + name;
    }

    SensorReadings* SensorReadings::instance;
    const std::map<RCP_DeviceClass_t, std::string> SensorReadings::DEVCLASS_NAMES = {};

    void SensorReadings::toCSVFile(SensorQualifier qual, std::vector<DataPoint>* data, std::atomic_bool* done) {
        if(std::filesystem::exists("exports")) {
            if(!std::filesystem::is_directory("./exports")) {
                *done = true;
                return;
            }
        }

        else std::filesystem::create_directory("./exports");

        const auto now = std::chrono::system_clock::now();
        std::ofstream file(std::format("./exports/{:%d-%m-%Y-%H-%M-%OS}-", now) + qual.asString() + ".csv");

        switch(qual.devclass) {
            case RCP_DEVCLASS_GPS:
                file << "relmillis,latitude,longitude,altitude,groundspeed\n";
                for(const auto& point : *data) {
                    file << std::format("{},{},{},{},{}\n", point.timestamp, point.data.gpsData[0],
                                        point.data.gpsData[1],
                                        point.data.gpsData[2], point.data.gpsData[3]);
                }
                break;

            case RCP_DEVCLASS_MAGNETOMETER:
            case RCP_DEVCLASS_ACCELEROMETER:
            case RCP_DEVCLASS_GYROSCOPE:
                file << "relmillis,x,y,z\n";
                for(const auto& point : *data) {
                    file << std::format("{},{},{},{}\n", point.timestamp, point.data.axisData[0],
                                        point.data.axisData[1],
                                        point.data.axisData[2]);
                }
                break;

            case RCP_DEVCLASS_AM_PRESSURE:
            case RCP_DEVCLASS_AM_TEMPERATURE:
            case RCP_DEVCLASS_PRESSURE_TRANSDUCER:
                file << "relmillis,data\n";
                for(const auto& point : *data) {
                    file << std::format("{},{}\n", point.timestamp, point.data.singleVal);
                }
                break;

            default:
                file << "Device Class not supported/recognized for export";
                break;
        }

        delete data;
        file.close();
        *done = true;
    }


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

                ImGui::SameLine();
                bool disable = filewritethreads.contains(qual) && filewritethreads[qual].thread;
                if(disable) ImGui::BeginDisabled();
                if(ImGui::Button("Write Data to CSV")) {
                    std::vector<DataPoint>* dcopy = new std::vector(data);
                    filewritethreads[qual].done = false;
                    filewritethreads[qual].thread = new std::thread(toCSVFile, qual, dcopy,
                                                                    &filewritethreads[qual].done);
                }
                if(disable) ImGui::EndDisabled();

                if(filewritethreads.contains(qual) && filewritethreads[qual].thread && filewritethreads[qual].done) {
                    filewritethreads[qual].thread->join();
                    delete filewritethreads[qual].thread;
                    filewritethreads[qual].thread = nullptr;
                    filewritethreads[qual].done = false;
                }

                ImGui::Text("Datapoints: %u", data.size());

                if(qual.devclass == RCP_DEVCLASS_GPS) {
                    if(ImPlot::BeginPlot((std::string("Sensor Data##") + qual.asString() + ":latlon").c_str(),
                                         plotsize)) {
                        ImPlot::SetupAxes("Time (s)", "Latitude/Longitude (degrees)");
                        ImPlot::SetupAxisLimits(ImAxis_X1, -1, 20);
                        ImPlot::SetupAxisLimits(ImAxis_Y1, -200, 200);
                        if(!data.empty())
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
                        ImPlot::SetupAxisLimits(ImAxis_X1, -1, 20);
                        ImPlot::SetupAxisLimits(ImAxis_Y1, 0, 100);
                        if(!data.empty())
                            ImPlot::PlotLine((std::string("Altitude##") + qual.asString()).c_str(), &data[0].timestamp,
                                             &data[0].data.gpsData[2], static_cast<int>(data.size()), 0, 0,
                                             sizeof(DataPoint));

                        ImPlot::EndPlot();
                    }

                    if(ImPlot::BeginPlot((std::string("Sensor Data##") + qual.asString() + ":gs").c_str(),
                                         plotsize)) {
                        ImPlot::SetupAxes("Time (s)", "Ground Speed (m/s)");
                        ImPlot::SetupAxisLimits(ImAxis_X1, -1, 20);
                        ImPlot::SetupAxisLimits(ImAxis_Y1, 0, 100);
                        if(!data.empty())
                            ImPlot::PlotLine((std::string("Ground Speed##") + qual.asString()).c_str(),
                                             &data[0].timestamp,
                                             &data[0].data.gpsData[3], static_cast<int>(data.size()), 0, 0,
                                             sizeof(DataPoint));

                        ImPlot::EndPlot();
                    }
                }

                else if(ImPlot::BeginPlot((std::string("Sensor Data##") + qual.asString()).c_str(), plotsize)) {
                    ImPlot::SetupAxisLimits(ImAxis_X1, -1, 20);
                    ImPlot::SetupAxisLimits(ImAxis_Y1, 0, 30);
                    // ImPlot::SetNextMarkerStyle(ImPlotMarker_Circle);
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

                        case RCP_DEVCLASS_RELATIVE_HYGROMETER:
                            ImPlot::SetupAxes("Time (s)", "Humidity (Relative %)");
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
                            case RCP_DEVCLASS_RELATIVE_HYGROMETER:
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

    void SensorReadings::reset() {
        for(auto& [qual, data] : sensors) {
            data.clear();
        }

        for(auto& [qual, thread] : filewritethreads) {
            if(thread.thread) {
                thread.thread->join();
                delete thread.thread;
            }
        }

        filewritethreads.clear();
    }

    void SensorReadings::setHardwareConfig(const std::set<SensorQualifier>& sensids) {
        reset();
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
