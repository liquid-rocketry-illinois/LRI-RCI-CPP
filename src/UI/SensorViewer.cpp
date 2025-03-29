#include <filesystem>
#include <ranges>

#include "UI/SensorViewer.h"
#include "implot.h"
#include "utils.h"

namespace LRI::RCI {
    int SensorViewer::CLASSID = 0;

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

    SensorViewer::SensorViewer(const std::set<HardwareQualifier>& quals, bool abridged) : classid(CLASSID++),
        abridged(abridged) {
        for(const auto& qual : quals) {
            sensors[qual] = Sensors::getInstance()->getState(qual);
        }
    }

    void SensorViewer::render() {
        ImGui::PushID("SensorViewer");
        ImGui::PushID(classid);

        ImDrawList* draw = ImGui::GetWindowDrawList();
        const float xsize = ImGui::GetWindowWidth() - scale(50);
        const ImVec2 plotsize = ImVec2(xsize, min3(xsize * (9.0f / 16.0f), scale(500),
                                                   ImGui::GetWindowHeight() - scale(75)));

        if(!ImGui::BeginChild("##child", ImGui::GetWindowSize() - scale(ImVec2(0, 40)))) {
            ImGui::EndChild();
            ImGui::PopID();
            ImGui::PopID();
            return;
        }

        if(abridged) {
            int num = 0;
            for(const auto& [qual, data] : sensors) {
                renderLatestReadings(qual, data->empty() ? empty : data->at(data->size() - 1));
                if(num++ % 4 != 0) {
                    ImGui::SameLine();
                    ImGui::Text(" | ");
                    ImGui::SameLine();
                }
            }

            ImGui::EndChild();

            ImGui::PopID();
            ImGui::PopID();
            return;
        }

        for(const auto& [qual, data] : sensors) {
            ImGui::PushID(qual.asString().c_str());

            if(!ImGui::TreeNode(qual.name.c_str())) {
                ImGui::PopID();
                continue;
            }

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
            if(ImGui::Button("Write To CSV"))
                Sensors::getInstance()->writeCSV(qual);

            renderGraphs(qual, data, plotsize);

            ImGui::Separator();
            ImGui::TreePop();

            ImGui::PopID();
        }

        ImGui::EndChild();

        ImGui::PopID();
        ImGui::PopID();
    }

    void SensorViewer::renderGraphs(const HardwareQualifier& qual, const std::vector<Sensors::DataPoint>* data,
                                    const ImVec2& plotsize) {
        for(const auto& graph : GRAPHINFO.at(qual.devclass)) {
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

            for(const auto& line : graph.lines) {
                ImPlot::PlotLine(line.name, &data->at(0).timestamp, (data->at(0).data + line.datanum),
                                 static_cast<int>(data->size()), 0, 0, sizeof(Sensors::DataPoint));
            }

            ImPlot::EndPlot();
            ImGui::PopID();
        }
    }

    // This map contains how each sensor should be rendered. It is quite a mess. clang-format is turned off
    // here since it looks neater this way than the way the formatter would make it

    // The map maps each sensor device class to a vector of graphs. Each graph contains the name of the graph,
    // what its Y axis should be (all X axes are time), and a vector of lines to be plotted. Each line contains
    // the name of the line, as well as the offset into the Sensors::DataPoint::Data array. It looks messy, but
    // this system is significantly cleaner than the previous implementation. As in half the number of LOC cleaner.
    // clang-format off
    const std::map<RCP_DeviceClass_t, std::vector<SensorViewer::Graph>> SensorViewer::test = {
        {RCP_DEVCLASS_AM_PRESSURE, {
            {"Ambient Pressure", "Pressure (mbars)", {{"Pressure", 0}}}
        }}};
    const std::map<RCP_DeviceClass_t, std::vector<SensorViewer::Graph>> SensorViewer::GRAPHINFO = {
        {RCP_DEVCLASS_AM_PRESSURE,         {{"Ambient Pressure",        "Pressure (mbars)",               {{"Pressure", 0}}}}},
        {RCP_DEVCLASS_AM_TEMPERATURE,      {{"Ambient Temperature",     "Temperature (Celsius)",          {{"Temperature", 0}}}}},
        {RCP_DEVCLASS_PRESSURE_TRANSDUCER, {{"Pressure",                "Pressure (psi)",                 {{"Pressure", 0}}}}},
        {RCP_DEVCLASS_RELATIVE_HYGROMETER, {{"Relative Humidity",       "Humidity (Relative %)",          {{"Humidity", 0}}}}},
        {RCP_DEVCLASS_LOAD_CELL,           {{"Load Cell",               "Mass (kg)",                      {{"Mass", 0}}}}},
        {RCP_DEVCLASS_POWERMON,            {{"Power Monitor - Voltage", "Voltage (V)",                    {{"Volts", 0}}}, {"Power Monitor - Power", "Power (W)", {{"Power", 1}}}}},
        {RCP_DEVCLASS_ACCELEROMETER,       {{"Accelerometer",           "Acceleration (m/s/s)",           {{"X", 0}, {"Y", 1}, {"Z", 2}}}}},
        {RCP_DEVCLASS_GYROSCOPE,           {{"Gyroscope",               "Angular Acceleration (deg/s/s)", {{"X", 0}, {"Y", 1}, {"Z", 2}}}}},
        {RCP_DEVCLASS_MAGNETOMETER,        {{"Magnetometer",            "Magnetic Field (Gauss)",         {{"X", 0}, {"Y", 1}, {"Z", 2}}}}},
        {RCP_DEVCLASS_GPS,                 {{"GPS - Lat & Lon",         "Position (degrees)",             {{"Latitude", 0}, {"Longitude", 1}}}, {"GPS - Altitude", "Altitude (m)", {{"Altitude", 2}}}, {"GPS - Ground Speed", "Speed (m/s)", {{"Speed", 3}}}}},
    };
    // clang-format on
}
