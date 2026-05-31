#include <filesystem>
#include <format>
#include <ranges>

#include "UI/MultiSensorViewer.h"
#include "implot.h"
#include "improgress.h"
#include "utils.h"

// Module for displaying sensor values. Most complicated viewer class
namespace LRI::RCI {
    namespace {
        /*
         * These two structures are used for encoding the naming and rendering information for
         * a particular device class. This data is stored in the GRAPHINFO map. For each
         * device class:
         *  - There is a vector holding a series of graphs. Multiple seperate graphs are needed
         *    in the case that a sensor returns different data types that cannot be graphed on the
         *    same set of axes (ie, gps data, power monitor data)
         *  - Each graph stores its name, its Y axis name (all X axes are time), and a vector of lines
         *  - Each line is an individual line on a graph. Up to 3 lines can be graphed per graph. This
         *    is used in the case that a sensor returns multiple values of the same unit that make sense
         *    to graph on the same set of axes, for example, an accelerometer
         *  - Each line stores its name, as well as which "data channel" of the sensor it represents. These
         *    correspond to one of the 4 doubles in the Sensors::DataPoint::data array, as each of the doubles
         *    represents a different channel of data from a particular sensor
         *
         * All this data is stored in the GRAPHINFO map, whose definition is present in the SensorViewer.cpp
         * file. It is truly a horrendous sight to behold, but its the best way I could think of to encode all
         * this data in the program. It does allow for some very clean loops that actually render the graphs,
         * though, as opposed to the previous system (in v1.0.x) which has a bunch of special cases for each
         * device class.
         */
        struct Line {
            const size_t axis = 0;
            const std::string legend = "{}";
        };

        struct GraphInfo {
            std::vector<std::string> axes;
            std::vector<Line> lines;
        };

        // clang-format off
        const std::map<const RCP_DeviceClass, const GraphInfo> GRAPHINFO {
            {RCP_DEVCLASS_MOTOR,               {{"Speed (rpm)"},                                                          {{0, "{}"}}}},
            {RCP_DEVCLASS_AM_PRESSURE,         {{"Pressure (mbars)"},                                                     {{0, "{}"}}}},
            {RCP_DEVCLASS_TEMPERATURE,         {{"Temperature (Celsius)"},                                                {{0, "{}"}}}},
            {RCP_DEVCLASS_PRESSURE_TRANSDUCER, {{"Pressure (psi)"},                                                       {{0, "{}"}}}},
            {RCP_DEVCLASS_RELATIVE_HYGROMETER, {{"Humidity (Relative %)"},                                                {{0, "{}"}}}},
            {RCP_DEVCLASS_FLOW_METER,          {{"Flow Rate (GPM)"},                                                      {{0, "{}"}}}},
            {RCP_DEVCLASS_LOAD_CELL,           {{"Mass (kg)"},                                                            {{0, "{}"}}}},
            {RCP_DEVCLASS_ALTITUDE,            {{"Altitude (m)"},                                                         {{0, "{}"}}}},
            {RCP_DEVCLASS_RADIO_STRENGTH,      {{"RSSI (dBm)"},                                                           {{0, "{}"}}}},
            {RCP_DEVCLASS_POWERMON,            {{"Voltage", "Power (W)"},                                              {{0, "{} Voltage"},  {1, "{} Power"}}}},
            {RCP_DEVCLASS_ACCELEROMETER,       {{"Acceleration (m/s/s)"},                                                 {{0, "{} X"},        {0, "{} Y"},         {0, "{} Z"}}}},
            {RCP_DEVCLASS_GYROSCOPE,           {{"Rotation (deg/s)"},                                                     {{0, "{} X"},        {0, "{} Y"},         {0, "{} Z"}}}},
            {RCP_DEVCLASS_MAGNETOMETER,        {{"Magnetic Field (Gauss)"},                                               {{0, "{} X"},        {0, "{} Y"},         {0, "{} Z"}}}},
            {RCP_DEVCLASS_RPY,                 {{"Orientation (degrees)"},                                                {{0, "{} Roll"},     {0, "{} Pitch"},     {0, "{} Yaw"}}}},
            {RCP_DEVCLASS_GPS,                 {{"Latitude", "Longitude", "Altitude (m)", "Ground Speed (m/s)"}, {{0, "{} Latitude"}, {1, "{} Longitude"}, {2, "{} Altitude"}, {3, "{} Ground Speed"}}}},
            {RCP_DEVCLASS_QUATERNION,          {{"Q"},                                                                    {{0, "{} W"}, {0, "{} X"}, {0, "{} Y"}, {0, "{} Z"}}}},
        };
        // clang-format on
    } // namespace

    // Helper
    static float min3(float a, float b, float c) { return std::min(a, std::min(b, c)); }

    std::map<HardwareQualifier, const std::vector<Sensors::DataPoint>*>
    MultiSensorViewer::extractSensors(const std::vector<GraphData>& graphs) {
        std::map<HardwareQualifier, const std::vector<Sensors::DataPoint>*> data;
        for(const auto& graph : graphs) {
            for(const auto& qual : graph.channels) {
                if(data.contains(qual)) continue;
                auto* datavec = Sensors::getState(qual);
                if(datavec != nullptr) data[qual] = datavec;
            }
        }

        return data;
    }

    std::map<HardwareChannel, std::string> MultiSensorViewer::extractLineNames(const std::vector<GraphData>& graphs) {
        std::map<HardwareChannel, std::string> lineNames;

        for(const auto& graph : graphs) {
            for(const auto& qual : graph.channels) {
                if(lineNames.contains(qual)) continue;
                lineNames[qual] = std::vformat(GRAPHINFO.at(qual.devclass).lines.at(qual.channel).legend, std::make_format_args(qual.name));
            }
        }

        return lineNames;
    }

    std::vector<std::string> MultiSensorViewer::extractTitles(const std::vector<GraphData>& graphs) {
        std::vector<std::string> titles;
        titles.reserve(graphs.size());
        for(const auto& graph : graphs) titles.emplace_back(graph.title);
        return titles;
    }

    std::vector<std::string> MultiSensorViewer::extractAxistList(const std::vector<GraphData>& graphs) {
        std::vector<std::string> axes;
        axes.reserve(graphs.size());

        for(const auto& graph : graphs) {
            if(graph.channels.empty()) continue;
            RCP_DeviceClass devclass = graph.channels[0].devclass;
            uint8_t channel = graph.channels[0].channel;
            axes.emplace_back(GRAPHINFO.at(devclass).axes.at(GRAPHINFO.at(devclass).lines.at(channel).axis));
        }

        return axes;
    }

    std::vector<std::vector<HardwareChannel>> MultiSensorViewer::extractChannels(const std::vector<GraphData>& graphs) {
        std::vector<std::vector<HardwareChannel>> channels;
        for(const auto& graph : graphs) channels.emplace_back(graph.channels);
        return channels;
    }

    // Store the abridged state
    // Add the qualifiers to track and their associated state pointer to the map
    MultiSensorViewer::MultiSensorViewer(const std::vector<GraphData>& quals) :
        data(std::move(extractSensors(quals))), lineNames(std::move(extractLineNames(quals))),
        titles(std::move(extractTitles(quals))), axislist(std::move(extractAxistList(quals))),
        channels(std::move(extractChannels(quals))) {}

    void MultiSensorViewer::render() {
        ImGui::PushID("SensorViewer");
        ImGui::PushID(classid);

        // Get the drawlist, and calculate the size of the plots
        const float xsize = ImGui::GetWindowWidth() - scale(25);
        const auto plotsize =
            ImVec2(xsize, min3(xsize * (9.0f / 16.0f), scale(500), ImGui::GetWindowHeight() - scale(25)));

        for(size_t index = 0; index < channels.size(); index++) {
            ImGui::PushID(titles.at(index).c_str());

            if(!ImPlot::BeginPlot(titles.at(index).c_str(), plotsize)) {
                ImGui::PopID();
                return;
            }

            ImPlot::SetupAxes("Time (s)", axislist.at(index).c_str(), ImPlotAxisFlags_AutoFit, ImPlotAxisFlags_AutoFit);

            for(const auto& qual : channels.at(index)) {
                if(data.at(qual)->empty()) ImPlot::PlotLine<double>(lineNames.at(qual).c_str(), nullptr, nullptr, 0, 0, 0, 0);
                else ImPlot::PlotLine(lineNames.at(qual).c_str(), &data.at(qual)->at(0).timestamp,
                                 data.at(qual)->at(0).data + qual.channel, static_cast<int>(data.at(qual)->size()), 0, 0,
                                 sizeof(Sensors::DataPoint));
            }

            ImPlot::EndPlot();
            ImGui::PopID();
        }

        ImGui::PopID();
        ImGui::PopID();
    }
} // namespace LRI::RCI
