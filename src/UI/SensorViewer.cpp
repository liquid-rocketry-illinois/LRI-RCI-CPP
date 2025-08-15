#include <filesystem>
#include <ranges>

#include "UI/SensorViewer.h"
#include "implot.h"
#include "improgress.h"
#include "utils.h"

// Module for displaying sensor values. Most complicated viewer class
namespace LRI::RCI {
    // Helper
    float min3(float a, float b, float c) { return std::min(a, std::min(b, c)); }

    // Format the latest datapoint as a string with the appropriate units
    std::string SensorViewer::renderLatestReadingsString(const HardwareQualifier& qual,
                                                         const Sensors::DataPoint& data) {
        switch(qual.devclass) {
        case RCP_DEVCLASS_AM_PRESSURE:
            return std::format("{}: {:.3f} mbar", qual.name, data.data[0]);

        case RCP_DEVCLASS_PRESSURE_TRANSDUCER:
            return std::format("{}: {:.3f} psi", qual.name, data.data[0]);

        case RCP_DEVCLASS_AM_TEMPERATURE:
            return std::format("{}: {:.3f} C", qual.name, data.data[0]);

        case RCP_DEVCLASS_RELATIVE_HYGROMETER:
            return std::format("{}: {:.3f} %", qual.name, data.data[0]);

        case RCP_DEVCLASS_LOAD_CELL:
            return std::format("{}: {:.3f} kg", qual.name, data.data[0]);

        case RCP_DEVCLASS_POWERMON:
            return std::format("{}: Voltage: {:.3f} V | Power: {:.3f} W", qual.name, data.data[0], data.data[1]);

        case RCP_DEVCLASS_ACCELEROMETER:
            return std::format("{}: X: {:.3f} m/s/s | Y: {:.3f} m/s/s | Z: {:.3f} m/s/s", qual.name, data.data[0],
                               data.data[1], data.data[2]);

        case RCP_DEVCLASS_GYROSCOPE:
            return std::format("{}: X: {:.3f} d/s/s | Y: {:.3f} d/s/s | Z: {:.3f} d/s/s", qual.name, data.data[0],
                               data.data[1], data.data[2]);

        case RCP_DEVCLASS_MAGNETOMETER:
            return std::format("{}: X: {:.3f} G | Y: {:.3f} G | Z: {:.3f} G", qual.name, data.data[0], data.data[1],
                               data.data[2]);

        case RCP_DEVCLASS_GPS:
            return std::format(
                "{}: Latitude: {:.3f} d | Longitude: {:.3f} d | Altitude: {:.3f} m | Ground Speed: {:.3f} m/s",
                qual.name, data.data[0], data.data[1], data.data[2], data.data[3]);

        default:
            return "Unrecognized sensor";
        }
    }

    // Store the abridged state
    // Add the qualifiers to track and their associated state pointer to the map
    SensorViewer::SensorViewer(const std::set<HardwareQualifier>& quals, bool abridged) : abridged(abridged) {
        for(const auto& qual : quals) {
            sensors[qual] = Sensors::getInstance()->getState(qual);
        }
    }

    void SensorViewer::render() {
        ImGui::PushID("SensorViewer");
        ImGui::PushID(classid);

        // Get the drawlist, and calculate the size of the plots
        ImDrawList* draw = ImGui::GetWindowDrawList();
        const float xsize = ImGui::GetWindowWidth() - scale(50);
        const auto plotsize =
            ImVec2(xsize, min3(xsize * (9.0f / 16.0f), scale(500), ImGui::GetWindowHeight() - scale(75)));

        // If in abridged mode, render the simpler stuff and exit early
        if(abridged) {
            float currentLineWidth = 0;
            // A lot of this math is just for text wrapping, so that numbers dont get wrapped in the middle
            const float width = ImGui::GetWindowWidth();
            const float spacerWidth = ImGui::CalcTextSize(" | ").x;
            for(const auto& [qual, data] : sensors) {
                std::string str = renderLatestReadingsString(qual, data->empty() ? empty : data->at(data->size() - 1));
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

            ImGui::PopID();
            ImGui::PopID();
            return;
        }

        // If not in abridged mode...
        if(!ImGui::BeginChild("##child", ImGui::GetWindowSize() - scale(ImVec2(0, 40)))) {
            ImGui::EndChild();
            ImGui::PopID();
            ImGui::PopID();
            return;
        }

        // Iterate through each qualifier and render its data
        for(const auto& [qual, data] : sensors) {
            ImGui::PushID(qual.asString().c_str());

            // Put them all in the little dropdown things
            if(!ImGui::TreeNode(qual.name.c_str())) {
                ImGui::PopID();
                continue;
            }

            // Status Square
            ImGui::Text("Sensor Status: ");
            ImGui::SameLine();
            ImVec2 pos = ImGui::GetCursorScreenPos();
            draw->AddRectFilled(pos, pos + scale(STATUS_SQUARE_SIZE), data->empty() ? STALE_COLOR : ENABLED_COLOR);
            ImGui::Dummy(scale(STATUS_SQUARE_SIZE));
            if(ImGui::IsItemHovered()) ImGui::SetTooltip(data->empty() ? "No data received" : "Receiving data");

            // Render the csv button, the current data point count, the current data point
            ImGui::SameLine();
            ImGui::Text(" | ");
            ImGui::SameLine();
            if(ImGui::Button("Write To CSV")) Sensors::getInstance()->writeCSV(qual);
            ImGui::SameLine();
            ImGui::Text(" | Data Points: %d", data->size());
            ImGui::TextWrapped("%s",
                        renderLatestReadingsString(qual, data->empty() ? empty : data->at(data->size() - 1)).c_str());
            if(!tarestate.contains(qual)) {
                tarestate[qual][0] = StopWatch();
                tarestate[qual][1] = StopWatch();
                tarestate[qual][2] = StopWatch();
                tarestate[qual][3] = StopWatch();
            }

            // Handle the tares. If the tarestate == -1, then no tare has been activated. If
            // the tare state is 0, 1, 2, or 3 then the first click to tare a data channel has been done,
            // and we're just waiting on the confirm
            ImGui::Text("Tare: ");
            float percent = 0.0f;
            for(const auto& graph : GRAPHINFO.at(qual.devclass)) {
                ImGui::PushID(graph.name);
                int i = 0;
                for(const auto& line : graph.lines) {
                    ImGui::PushID(i++);
                    ImGui::SameLine();
                    if(ImGui::TimedButton(line.name, tarestate[qual][line.datanum])) {
                        percent = tarestate[qual][line.datanum].timeSince() / CONFIRM_HOLD_TIME;
                        if(percent >= 1.0f) {
                            Sensors::getInstance()->tare(qual, line.datanum);
                            tarestate[qual][line.datanum].reset();
                        }
                    }
                    ImGui::PopID();
                }
                ImGui::PopID();
            }

            ImGui::SameLine();
            ImGui::CircleProgressBar("##tareprogressspinner", 10, 3, WHITE_COLOR, percent);

            if(ImGui::TimedButton("Clear Graphs", clearState[qual])) {
                ImGui::SameLine();
                ImGui::CircleProgressBar("##clearprogressspinner", 10, 3, WHITE_COLOR,
                                         clearState[qual].timeSince() / CONFIRM_HOLD_TIME);
                if(clearState[qual].timeSince() > CONFIRM_HOLD_TIME) {
                    Sensors::getInstance()->clearGraph(qual);
                    clearState[qual].reset();
                }
            }

            // Render the graph itself
            renderGraphs(qual, data, plotsize);

            ImGui::Separator();
            ImGui::TreePop();

            ImGui::PopID();
        }

        ImGui::EndChild();

        ImGui::PopID();
        ImGui::PopID();
    }

    // Helper for rendering graphs
    void SensorViewer::renderGraphs(const HardwareQualifier& qual, const std::vector<Sensors::DataPoint>* data,
                                    const ImVec2& plotsize) {
        // See SensorViewer.h for details on the structure of GRAPHINFO
        for(const auto& graph : GRAPHINFO.at(qual.devclass)) {
            // Iterate through each graph, set up its axis, blah blah
            ImGui::PushID(graph.name);
            if(!ImPlot::BeginPlot(graph.name, plotsize)) {
                ImGui::PopID();
                continue;
            }

            ImPlot::SetupAxes("Time (s)", graph.axis, ImPlotAxisFlags_AutoFit, ImPlotAxisFlags_AutoFit);
            if(data->empty()) {
                ImPlot::EndPlot();
                ImGui::PopID();
                continue;
            }

            // Iterate through each line and render it
            for(const auto& line : graph.lines) {
                ImPlot::PlotLine(line.name, &data->at(0).timestamp, (data->at(0).data + line.datanum),
                                 static_cast<int>(data->size()), 0, 0, sizeof(Sensors::DataPoint));
            }

            ImPlot::EndPlot();
            ImGui::PopID();
        }
    }


    // See SensorViewer.h. Clang format is turned off since it makes this look more like a mess than manually
    // formatting it

    // clang-format off
    const std::map<RCP_DeviceClass, std::vector<SensorViewer::Graph>> SensorViewer::GRAPHINFO = {
        {RCP_DEVCLASS_AM_PRESSURE,         {{"Ambient Pressure",        "Pressure (mbars)",               {{"Pressure", 0}}}}},
        {RCP_DEVCLASS_AM_TEMPERATURE,      {{"Ambient Temperature",     "Temperature (Celsius)",          {{"Temperature", 0}}}}},
        {RCP_DEVCLASS_PRESSURE_TRANSDUCER, {{"Pressure",                "Pressure (psi)",                 {{"Pressure", 0}}}}},
        {RCP_DEVCLASS_RELATIVE_HYGROMETER, {{"Relative Humidity",       "Humidity (Relative %)",          {{"Humidity", 0}}}}},
        {RCP_DEVCLASS_LOAD_CELL,           {{"Load Cell",               "Mass (kg)",                      {{"Mass", 0}}}}},
        {RCP_DEVCLASS_POWERMON,            {{"Power Monitor - Voltage", "Voltage (V)",                    {{"Volts", 0}}}, {"Power Monitor - Power", "Power (W)", {{"Power", 1}}}}},
        {RCP_DEVCLASS_ACCELEROMETER,       {{"Accelerometer",           "Acceleration (m/s/s)",           {{"X", 0}, {"Y", 1}, {"Z", 2}}}}},
        {RCP_DEVCLASS_GYROSCOPE,           {{"Gyroscope",               "Rotation (deg/s)", {{"X", 0}, {"Y", 1}, {"Z", 2}}}}},
        {RCP_DEVCLASS_MAGNETOMETER,        {{"Magnetometer",            "Magnetic Field (Gauss)",         {{"X", 0}, {"Y", 1}, {"Z", 2}}}}},
        {RCP_DEVCLASS_GPS,                 {{"GPS - Lat & Lon",         "Position (degrees)",             {{"Latitude", 0}, {"Longitude", 1}}}, {"GPS - Altitude", "Altitude (m)", {{"Altitude", 2}}}, {"GPS - Ground Speed", "Speed (m/s)", {{"Speed", 3}}}}},
    };
    // clang-format on
} // namespace LRI::RCI
