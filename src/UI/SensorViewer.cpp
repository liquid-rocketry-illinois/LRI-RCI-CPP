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

    // Store the abridged state
    // Add the qualifiers to track and their associated state pointer to the map
    SensorViewer::SensorViewer(const std::vector<HardwareQualifier>& quals, bool showControls) :
        showControls(showControls) {
        for(const auto& qual : quals) {
            if(qual.devclass == RCP_DEVCLASS_TEST_STATE) {
                sensors.emplace_back(qual, nullptr);
                continue;
            }

            const auto* sense = Sensors::getState(qual);
            if(sense == nullptr) continue;
            sensors.emplace_back(qual, sense);
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

        if(showControls) {
            if(ImGui::TimedButton("Clear All Graphs", clearAllTimer)) {
                ImGui::SameLine();
                ImGui::CircleProgressBar("##clearallprogressspinner", 10, 3, WHITE_COLOR,
                                         clearAllTimer.timeSince() / CONFIRM_HOLD_TIME);
                if(clearAllTimer.timeSince() > CONFIRM_HOLD_TIME) {
                    for(const auto& qual : sensors | std::views::keys) Sensors::clearGraph(qual);
                    clearAllTimer.reset();
                }
            }

            if(ImGui::TimedButton("Tare All Devices", tareAllTimer)) {
                ImGui::SameLine();
                ImGui::CircleProgressBar("##tareallprogressspinner", 10, 3, WHITE_COLOR,
                                         tareAllTimer.timeSince() / CONFIRM_HOLD_TIME);
                if(tareAllTimer.timeSince() > CONFIRM_HOLD_TIME) {
                    for(const auto& qual : sensors | std::views::keys) {
                        for(const auto& graph : GRAPHINFO.at(qual.devclass)) {
                            // This WILL NOT WORK for devclasses that have multiple graphs displaying the same datanum.
                            // WONTFIX, proper version will be in v2 builds
                            for(const auto& line : graph.lines) Sensors::tare(qual, line.datanum);
                        }
                    }
                    tareAllTimer.reset();
                }
            }

            if(ImGui::Button("Write all to CSV")) {
                for(const auto& qual : sensors | std::views::keys) Sensors::writeCSV(qual);
            }
        }

        // Iterate through each qualifier and render its data
        for(const auto& [qual, data] : sensors) {
            ImGui::PushID(qual.asString().c_str());

            // Put them all in the little dropdown things
            if(!ImGui::TreeNode(qual.name.c_str())) {
                ImGui::PopID();
                continue;
            }

            if(showControls) {

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
                if(ImGui::Button("Write To CSV")) Sensors::writeCSV(qual);
                ImGui::SameLine();
                ImGui::Text(" | Data Points: %lld", data->size());
                ImGui::TextWrapped("%s",
                                   Sensors::renderLatestReadingsString(
                                       qual, data->empty() ? Sensors::empty : data->at(data->size() - 1))
                                       .c_str());
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
                                Sensors::tare(qual, line.datanum);
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
                        Sensors::clearGraph(qual);
                        clearState[qual].reset();
                    }
                }
            }

            // Render the graph itself
            renderGraphs(qual, data, plotsize);

            ImGui::Separator();
            ImGui::TreePop();

            ImGui::PopID();
        }

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
        {RCP_DEVCLASS_MOTOR,               {{"Motor",                   "Speed (rpm)",                    {{"Speed", 0}}}}},
        {RCP_DEVCLASS_ANGLED_ACTUATOR,     {{"Actuator Angle",          "Angle (Degrees)",                {{"Angle", 0}}}}},
        {RCP_DEVCLASS_AM_PRESSURE,         {{"Ambient Pressure",        "Pressure (mbars)",               {{"Pressure", 0}}}}},
        {RCP_DEVCLASS_TEMPERATURE,         {{"Temperature",             "Temperature (Celsius)",          {{"Temperature", 0}}}}},
        {RCP_DEVCLASS_PRESSURE_TRANSDUCER, {{"Pressure",                "Pressure (psi)",                 {{"Pressure", 0}}}}},
        {RCP_DEVCLASS_RELATIVE_HYGROMETER, {{"Relative Humidity",       "Humidity (Relative %)",          {{"Humidity", 0}}}}},
        {RCP_DEVCLASS_LOAD_CELL,           {{"Load Cell",               "Mass (kg)",                      {{"Mass", 0}}}}},
        {RCP_DEVCLASS_ALTITUDE,            {{"Altitude",                "Altitude (m)",                   {{"Altitude", 0}}}}},
        {RCP_DEVCLASS_RADIO_STRENGTH,      {{"Radio Strength",          "Strength (dBm)",                 {{"Strength", 0}}}}},
        {RCP_DEVCLASS_POWERMON,            {{"Power Monitor - Voltage", "Voltage (V)",                    {{"Volts", 0}}}, {"Power Monitor - Power", "Power (W)", {{"Power", 1}}}}},
        {RCP_DEVCLASS_ACCELEROMETER,       {{"Accelerometer",           "Acceleration (m/s/s)",           {{"X", 0}, {"Y", 1}, {"Z", 2}}}}},
        {RCP_DEVCLASS_GYROSCOPE,           {{"Gyroscope",               "Rotation (deg/s)",               {{"X", 0}, {"Y", 1}, {"Z", 2}}}}},
        {RCP_DEVCLASS_MAGNETOMETER,        {{"Magnetometer",            "Magnetic Field (Gauss)",         {{"X", 0}, {"Y", 1}, {"Z", 2}}}}},
        {RCP_DEVCLASS_RPY,                 {{"Orientation",             "Orientation (degrees)",          {{"Roll", 0}, {"Pitch", 1}, {"Yaw", 2}}}}},
        {RCP_DEVCLASS_GPS,                 {{"GPS - Lat & Lon",         "Position (degrees)",             {{"Latitude", 0}, {"Longitude", 1}}}, {"GPS - Altitude", "Altitude (m)", {{"Altitude", 2}}}, {"GPS - Ground Speed", "Speed (m/s)", {{"Speed", 3}}}}},
        {RCP_DEVCLASS_QUATERNION,          {{"Quaternion",              "Value",                          {{"W", 0}, {"X", 1}, {"Y", 2}, {"Z", 3}}}}},
        {RCP_DEVCLASS_FLOW_METER,          {{"Flow Meter",              "Flow Rate (GPM)",                {{"Flow Rate", 0}}}}},
    };
    // clang-format on
} // namespace LRI::RCI
