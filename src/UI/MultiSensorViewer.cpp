#include <filesystem>
#include <format>
#include <ranges>

#include "UI/MultiSensorViewer.h"
#include "UI/gutils.h"
#include "hardware/hwctrl.h"
#include "implot.h"
#include "improgress.h"

// Module for displaying sensor values. Most complicated viewer class
namespace LRI::RCI {


    // Helper
    static float min3(float a, float b, float c) { return std::min(a, std::min(b, c)); }

    std::vector<MultiSensorViewer::GraphMeta> MultiSensorViewer::makeGraphMeta(const std::vector<GraphData>& graphs) {
        std::vector<GraphMeta> gms;
        for(const auto& graph : graphs) {
            if(graph.channels.empty()) {
                hwctrl::addError(Error::LWARNING, "Empty multisensors graph");
                continue;
            }

            GraphMeta gm;
            gm.title = graph.title;
            gm.channels = graph.channels; // All entries in graph.channels should have the same devclass and channel
            const auto& ch = graph.channels.front();

            const size_t axis = GraphInfo::GRAPHINFO.at(ch.devclass).lines.at(ch.channel).axis;
            gm.axis = GraphInfo::GRAPHINFO.at(ch.devclass).axes.at(axis).name;
        }

        return gms;
    }

    std::map<HardwareChannel, MultiSensorViewer::ChannelData>
    MultiSensorViewer::makeChannelData(const std::vector<GraphData>& graphs) {
        std::map<HardwareChannel, ChannelData> data;
        for(const auto& graph : graphs) {
            for(const auto& ch : graph.channels) {
                if(data.contains(ch)) continue;
                data[ch] = {
                    hwctrl::getELog()->getChannelFloatData(ch),
                    std::format("{} {}", ch.name, GraphInfo::GRAPHINFO.at(ch.devclass).lines.at(ch.channel).legend)};
            }
        }

        return data;
    }

    // Store the abridged state
    // Add the qualifiers to track and their associated state pointer to the map
    MultiSensorViewer::MultiSensorViewer(const std::vector<GraphData>& quals) :
        graphs(makeGraphMeta(quals)), channelData(makeChannelData(quals)) {}

    void MultiSensorViewer::render() {
        ImGui::PushID("SensorViewer");
        ImGui::PushID(classid);

        // Get the drawlist, and calculate the size of the plots
        const float xsize = ImGui::GetWindowWidth() - 27_sc;
        const auto plotsize = ImVec2(xsize, min3(xsize * (9.0f / 16.0f), 500_sc, ImGui::GetWindowHeight() - 25_sc));

        for(size_t i = 0; i < graphs.size(); i++) {
            ImGui::PushID(static_cast<int>(i));

            const auto& graph = graphs[i];

            if(!ImPlot::BeginPlot(graph.title.c_str(), plotsize)) {
                ImGui::PopID();
                continue;
            }

            ImPlot::SetupAxes("Time (s)", graph.axis.c_str(), ImPlotAxisFlags_AutoFit, ImPlotAxisFlags_AutoFit);
            for(const auto& channel : graph.channels) {
                const auto& line = channelData.at(channel);
                ImPlot::PlotLineG(line.lineName.c_str(), implotTargetFloat,
                                  const_cast<EventLog::TargetFloat*>(&line.data),
                                  static_cast<int>(line.data.times->size()));
            }

            ImPlot::EndPlot();
            ImGui::PopID();
        }

        ImGui::PopID();
        ImGui::PopID();
    }
} // namespace LRI::RCI
