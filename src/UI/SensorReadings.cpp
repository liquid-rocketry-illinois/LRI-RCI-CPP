#include <utility>
#include <fstream>
#include <filesystem>
#include <chrono>
#include <format>

#include "UI/SensorReadings.h"
#include <implot.h>

namespace LRI::RCI {
    // This is used for sorting the sensors in the list on the display
    bool SensorQualifier::operator<(SensorQualifier const& rhf) const {
        if(devclass == rhf.devclass) return id < rhf.id;
        return devclass < rhf.devclass;
    }

    // Used in imgui IDs, not necessarily meant for display. In the format of DEVCLASS-ID-NAME
    std::string SensorQualifier::asString() const {
        return std::to_string(devclass) + "-" + std::to_string(id) + "-" + name;
    }

    SensorReadings* SensorReadings::instance;

    // Function that gets run in thread to put sensor data into a csv. The vector of data is de-allocated at the end!
    void SensorReadings::toCSVFile(const SensorQualifier& qual, const std::vector<DataPoint>* data,
                                   std::atomic_bool* done) {
        // Create the exports directory if it does not exist. If it exists as a file, exit early
        if(std::filesystem::exists("exports")) {
            if(!std::filesystem::is_directory("./exports")) {
                *done = true;
                return;
            }
        }

        else std::filesystem::create_directory("./exports");

        const auto now = std::chrono::system_clock::now();
        std::ofstream file(std::format("./exports/{:%d-%m-%Y-%H-%M-%OS}-", now) + qual.asString() + ".csv");

        // The actual file writing. Depending on the device class, a header is created, and each datapoint is iterated
        // through and written to the file
        switch(qual.devclass) {
        case RCP_DEVCLASS_GPS:
            file << "relmillis,latitude,longitude,altitude,groundspeed\n";
            for(const auto& point : *data) {
                file << std::format("{},{},{},{},{}\n", point.timestamp, point.data[0],
                                    point.data[1],
                                    point.data[2], point.data[3]);
            }
            break;

        case RCP_DEVCLASS_MAGNETOMETER:
        case RCP_DEVCLASS_ACCELEROMETER:
        case RCP_DEVCLASS_GYROSCOPE:
            file << "relmillis,x,y,z\n";
            for(const auto& point : *data) {
                file << std::format("{},{},{},{}\n", point.timestamp, point.data[0],
                                    point.data[1],
                                    point.data[2]);
            }
            break;

        case RCP_DEVCLASS_AM_PRESSURE:
        case RCP_DEVCLASS_AM_TEMPERATURE:
        case RCP_DEVCLASS_PRESSURE_TRANSDUCER:
            file << "relmillis,data\n";
            for(const auto& point : *data) {
                file << std::format("{},{}\n", point.timestamp, point.data[0]);
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

    // Helper
    float min3(float a, float b, float c) {
        return min(a, min(b, c));
    }

    void SensorReadings::renderLatestReadings(const SensorQualifier& qual, const DataPoint& data) {
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

    void SensorReadings::drawSensors() {
        ImDrawList* draw = ImGui::GetWindowDrawList();
        float xsize = ImGui::GetWindowWidth() - scale(50);

        ImVec2 plotsize = ImVec2(xsize,
                                 min3(xsize * (9.0f / 16.0f), scale(500), ImGui::GetWindowHeight() - scale(75)));

        if(!ImGui::BeginChild("##sensorchild", ImGui::GetWindowSize() - scale(
                                  fullscreen ? ImVec2(0, 30) : ImVec2(0, 50)))) {
            ImGui::EndChild();
            return;
        }
        // Iterate through every sensor in the list to graph
        for(const auto& [qual, data] : sensors) {
            // Tree nodes to keep things organized
            if(!ImGui::TreeNode(
                (qual.name + "##" + qual.asString()).c_str()))
                continue;

            // Status square
            ImGui::Text("Sensor Status: ");
            ImGui::SameLine();
            ImVec2 pos = ImGui::GetCursorScreenPos();
            draw->AddRectFilled(pos, pos + scale(STATUS_SQUARE_SIZE),
                                data.empty() ? STALE_COLOR : ENABLED_COLOR);
            ImGui::Dummy(scale(STATUS_SQUARE_SIZE));
            if(ImGui::IsItemHovered()) ImGui::SetTooltip(data.empty() ? "No data received" : "Receiving data");

            // This all is for the button to write the data to a file. With datasets of around 10,000 points this
            // operation can take some time, hence why it is done on a seperate thread. The data vector is copied
            // to a new vector, so that the thread has its own copy that is not changing. It is deleted by the
            // thread once the file operation is done.
            ImGui::SameLine();
            ImGui::Text(" | ");
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

            if(filewritethreads.contains(qual) && filewritethreads[qual].thread &&
                filewritethreads[qual].done) {
                filewritethreads[qual].thread->join();
                delete filewritethreads[qual].thread;
                filewritethreads[qual].thread = nullptr;
                filewritethreads[qual].done = false;
            }

            // Simple helper for seeing how many datapoints are present. Once it gets over ~200k points, it starts
            // becoming a little laggy. Not much you can do there, it's just a fault of rendering 200k line segments
            ImGui::SameLine();
            ImGui::Text(" | ");
            ImGui::SameLine();
            ImGui::Text("Datapoints: %u", static_cast<unsigned int>(data.size()));
            renderLatestReadings(qual, data.empty()
                                           ? DataPoint{.timestamp = 0, .data = {0, 0, 0, 0}}
                                           : data[data.size() - 1]);

            // Graphing is done by giving implot a pointer to the beginning of each data vector. From there, it is
            // also given offsets to the timestamp value of each datapoint, as well as the appropriate type from the
            // union inside each datapoint. It is also given a "stride" value (the last argument in the PlotLine
            // function), which tells implot how many bytes in memory to step over to get to the next piece of data.
            // This is useful since the datapoint struct technically stores 4 different values in the union, so
            // the stride lets us tell implot how much to skip over for each datapoint.

            // The gps is special, since it has 4 different data types in one sensor, hence why it gets its own
            // section
            if(qual.devclass == RCP_DEVCLASS_GPS) {
                if(ImPlot::BeginPlot((std::string("Sensor Data##") + qual.asString() + ":latlon").c_str(),
                                     plotsize)) {
                    ImPlot::SetupAxes("Time (s)", "Latitude/Longitude (degrees)", ImPlotAxisFlags_AutoFit,
                                      ImPlotAxisFlags_AutoFit);
                    if(!data.empty())
                        ImPlot::PlotLine((std::string("Latitude##") + qual.asString()).c_str(),
                                         &data[0].timestamp,
                                         data[0].data, static_cast<int>(data.size()), 0, 0,
                                         sizeof(DataPoint));

                    if(!data.empty())
                        ImPlot::PlotLine((std::string("Longitude##") + qual.asString()).c_str(),
                                         &data[0].timestamp,
                                         data[0].data + 1, static_cast<int>(data.size()), 0, 0,
                                         sizeof(DataPoint));

                    ImPlot::EndPlot();
                }

                if(ImPlot::BeginPlot((std::string("Sensor Data##") + qual.asString() + ":alt").c_str(),
                                     plotsize)) {
                    ImPlot::SetupAxes("Time (s)", "Altitude (m)", ImPlotAxisFlags_AutoFit, ImPlotAxisFlags_AutoFit);
                    if(!data.empty())
                        ImPlot::PlotLine((std::string("Altitude##") + qual.asString()).c_str(),
                                         &data[0].timestamp,
                                         data[0].data + 2, static_cast<int>(data.size()), 0, 0,
                                         sizeof(DataPoint));

                    ImPlot::EndPlot();
                }

                if(ImPlot::BeginPlot((std::string("Sensor Data##") + qual.asString() + ":gs").c_str(),
                                     plotsize)) {
                    ImPlot::SetupAxes("Time (s)", "Ground Speed (m/s)", ImPlotAxisFlags_AutoFit,
                                      ImPlotAxisFlags_AutoFit);
                    if(!data.empty())
                        ImPlot::PlotLine((std::string("Ground Speed##") + qual.asString()).c_str(),
                                         &data[0].timestamp,
                                         data[0].data + 3, static_cast<int>(data.size()), 0, 0,
                                         sizeof(DataPoint));

                    ImPlot::EndPlot();
                }
            }

            else if(qual.devclass == RCP_DEVCLASS_POWERMON) {
                if(ImPlot::BeginPlot((std::string("Sensor Data##") + qual.asString() + ":volt").c_str(), plotsize)) {
                    ImPlot::SetupAxes("Time(s)", "Voltage (V)", ImPlotAxisFlags_AutoFit, ImPlotAxisFlags_AutoFit);
                    if(!data.empty())
                        ImPlot::PlotLine((std::string("Voltage##") + qual.asString()).c_str(),
                                         &data[0].timestamp,
                                         data[0].data, static_cast<int>(data.size()), 0, 0,
                                         sizeof(DataPoint));
                }

                if(ImPlot::BeginPlot((std::string("Sensor Data##") + qual.asString() + ":pow").c_str(), plotsize)) {
                    ImPlot::SetupAxes("Time(s)", "Power (W)", ImPlotAxisFlags_AutoFit, ImPlotAxisFlags_AutoFit);
                    if(!data.empty())
                        ImPlot::PlotLine((std::string("Power##") + qual.asString()).c_str(),
                                         &data[0].timestamp,
                                         data[0].data + 1, static_cast<int>(data.size()), 0, 0,
                                         sizeof(DataPoint));
                }
            }

            // For every other sensor that just has one type, they are handled here. Sensors with 3 axis data
            // (accelerometer, magnetometer, gyroscope) have all data on one plot, but with 3 different lines.
            else if(ImPlot::BeginPlot((std::string("Sensor Data##") + qual.asString()).c_str(), plotsize)) {
                // The below line can be uncommented to enable markers on each datapoint, however this drastically
                // decreases performance to a max of around ~75k points
                // ImPlot::SetNextMarkerStyle(ImPlotMarker_Circle);

                // Two switches happen. The first is to set the axis names to the correct datatype, since this is
                // unique to each sensor
                std::string graphname;
                switch(qual.devclass) {
                case RCP_DEVCLASS_PRESSURE_TRANSDUCER:
                    ImPlot::SetupAxes("Time (s)", "Pressure (psi)", ImPlotAxisFlags_AutoFit, ImPlotAxisFlags_AutoFit);
                    graphname = "Pressure##";
                    break;

                case RCP_DEVCLASS_AM_PRESSURE:
                    ImPlot::SetupAxes("Time (s)", "Pressure (millibars)", ImPlotAxisFlags_AutoFit,
                                      ImPlotAxisFlags_AutoFit);
                    graphname = "Pressure##";
                    break;

                case RCP_DEVCLASS_MAGNETOMETER:
                    ImPlot::SetupAxes("Time (s)", "Magnetic Strength (Gauss)", ImPlotAxisFlags_AutoFit,
                                      ImPlotAxisFlags_AutoFit);
                    break;

                case RCP_DEVCLASS_AM_TEMPERATURE:
                    ImPlot::SetupAxes("Time (s)", "Temperature (Celcius)", ImPlotAxisFlags_AutoFit,
                                      ImPlotAxisFlags_AutoFit);
                    graphname = "Temperature##";
                    break;

                case RCP_DEVCLASS_ACCELEROMETER:
                    ImPlot::SetupAxes("Time (s)", "Acceleration (m/s/s)", ImPlotAxisFlags_AutoFit,
                                      ImPlotAxisFlags_AutoFit);
                    break;

                case RCP_DEVCLASS_GYROSCOPE:
                    ImPlot::SetupAxes("Time (s)", "Angular Acceleration (d/s/s)", ImPlotAxisFlags_AutoFit,
                                      ImPlotAxisFlags_AutoFit);
                    break;

                case RCP_DEVCLASS_RELATIVE_HYGROMETER:
                    ImPlot::SetupAxes("Time (s)", "Humidity (Relative %)", ImPlotAxisFlags_AutoFit,
                                      ImPlotAxisFlags_AutoFit);
                    break;

                case RCP_DEVCLASS_LOAD_CELL:
                    ImPlot::SetupAxes("Time (s)", "Mass (kg)", ImPlotAxisFlags_AutoFit, ImPlotAxisFlags_AutoFit);
                    break;

                default:
                    ImPlot::SetupAxes("Unknown Data", "Unknown Data", ImPlotAxisFlags_AutoFit, ImPlotAxisFlags_AutoFit);
                    break;
                }

                // The second switch (which only happens when data is present) actually plots the graphs. This is
                // done since several different sensors are graphed in the same way, and just have different axis
                // names and units
                if(!data.empty())
                    switch(qual.devclass) {
                    case RCP_DEVCLASS_PRESSURE_TRANSDUCER:
                    case RCP_DEVCLASS_AM_PRESSURE:
                    case RCP_DEVCLASS_AM_TEMPERATURE:
                    case RCP_DEVCLASS_RELATIVE_HYGROMETER:
                    case RCP_DEVCLASS_LOAD_CELL:
                        ImPlot::PlotLine((graphname + qual.asString()).c_str(), &data[0].timestamp,
                                         data[0].data, static_cast<int>(data.size()),
                                         0, 0, sizeof(DataPoint));
                        break;

                    case RCP_DEVCLASS_MAGNETOMETER:
                    case RCP_DEVCLASS_ACCELEROMETER:
                    case RCP_DEVCLASS_GYROSCOPE:
                        ImPlot::PlotLine((std::string("X##") + qual.asString()).c_str(), &data[0].timestamp,
                                         data[0].data, static_cast<int>(data.size()), 0, 0,
                                         sizeof(DataPoint));

                        ImPlot::PlotLine((std::string("Y##") + qual.asString()).c_str(), &data[0].timestamp,
                                         data[0].data + 1, static_cast<int>(data.size()), 0, 0,
                                         sizeof(DataPoint));

                        ImPlot::PlotLine((std::string("Z##") + qual.asString()).c_str(), &data[0].timestamp,
                                         data[0].data + 2, static_cast<int>(data.size()), 0, 0,
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

        ImGui::EndChild();
    }

    void SensorReadings::render() {
        ImGui::SetNextWindowSize(scale(ImVec2(700, 350)), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowPos(scale(ImVec2(675, 50)), ImGuiCond_FirstUseEver);

        if(doResize) {
            if(fullscreen) {
                ImGui::SetNextWindowPos(ImVec2());
                ImGui::SetNextWindowSize(ImGui::GetMainViewport()->Size);
            }

            else {
                ImGui::SetNextWindowPos(preFullscreeenPos);
                ImGui::SetNextWindowSize(preFullscreenSize);
            }

            doResize = false;
        }

        if(ImGui::Begin("Sensor Readouts", nullptr, fullscreen ? fullscreenFlags : regularFlags)) {
            drawSensors();
            if(ImGui::Button("Fullscreen")) {
                fullscreen = !fullscreen;
                doResize = true;

                if(fullscreen) {
                    preFullscreenSize = ImGui::GetWindowSize();
                    preFullscreeenPos = ImGui::GetWindowPos();
                }
            }
        }

        ImGui::End();
    }

    void SensorReadings::reset() {
        // Clear all sensors from the list
        for(auto& [qual, data] : sensors) {
            data.clear();
        }

        // Make sure no more file writing threads are active. Then exit
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

    void SensorReadings::receiveRCPUpdate(const RCP_OneFloat& data) {
        SensorQualifier qual = {.devclass = data.devclass, .id = data.ID};
        DataPoint d = {
            .timestamp = static_cast<double>(data.timestamp) / 1'000.0,
            .data = {static_cast<double>(data.data)}
        };

        sensors[qual].push_back(d);
    }

    void SensorReadings::receiveRCPUpdate(const RCP_TwoFloat& data) {
        SensorQualifier qual = {.devclass = data.devclass, .id = data.ID};
        DataPoint d = {.timestamp = static_cast<double>(data.timestamp) / 1'000.0};
        for(int i = 0; i < 2; i++) d.data[i] = static_cast<double>(data.data[i]);
        sensors[qual].push_back(d);
    }

    void SensorReadings::receiveRCPUpdate(const RCP_ThreeFloat& data) {
        SensorQualifier qual = {.devclass = data.devclass, .id = data.ID};
        DataPoint d = {.timestamp = static_cast<double>(data.timestamp) / 1'000.0};
        for(int i = 0; i < 3; i++) d.data[i] = static_cast<double>(data.data[i]);
        sensors[qual].push_back(d);
    }

    void SensorReadings::receiveRCPUpdate(const RCP_FourFloat& data) {
        SensorQualifier qual = {.devclass = data.devclass, .id = data.ID};
        DataPoint d = {.timestamp = static_cast<double>(data.timestamp) / 1'000.0};
        for(int i = 0; i < 4; i++) d.data[i] = static_cast<double>(data.data[i]);
        sensors[qual].push_back(d);
    }
}
