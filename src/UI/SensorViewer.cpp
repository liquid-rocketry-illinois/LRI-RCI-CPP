#include <filesystem>
#include <ranges>

#include "UI/SensorViewer.h"

#include "UI/gutils.h"
#include "hardware/EventLog.h"
#include "hardware/hwctrl.h"
#include "imgui_internal.h"
#include "implot.h"
#include "implot_internal.h"
#include "improgress.h"

// Module for displaying sensor values. Most complicated viewer class
namespace LRI::RCI {
    // Helper
    float min3(float a, float b, float c) { return std::min(a, std::min(b, c)); }

    static float calculateTare(EventLog::TargetFloat data) {
        if(data.times->empty()) return 0;

        // Start the average 1 second prior in the data
        double startTime = data.times->back().ttime - 1000;

        // Determine where in the data we need to start: average all data within the last second
        size_t startIndex = data.times->size() - 1;
        while(data.times->at(startIndex).ttime > startTime && startIndex > 0) startIndex--;

        // To get the average, we will sum all the values from startIndex to the end of the array. We need the total
        // elements we are summing, as well as their total value
        size_t numElems = data.times->size() - startIndex;

        float total = 0;
        for(; startIndex < data.values->size(); startIndex++) total += data.values->at(startIndex);

        // Average is just total / # elems
        // Must negate the value: RCP spec mandates the value we pass to RCP_requestTareConfiguration is ADDED to all
        // future data readings, and we are wanting to subtract off the amount we calculate here to get back to zero
        return -total / static_cast<float>(numElems);
    }

    std::map<HardwareQualifier, std::vector<EventLog::TargetFloat>>
    SensorViewer::getELogData(const std::vector<HardwareQualifier>& quals) {
        std::map<HardwareQualifier, std::vector<EventLog::TargetFloat>> fdata;

        for(const auto& qual : quals) {
            if(fdata.contains(qual)) continue;
            fdata[qual] = hwctrl::getELog()->getAllChannels(qual);
        }

        return fdata;
    }

    SensorViewer::SensorViewer(const std::vector<HardwareQualifier>& quals, bool showControls) :
        showControls(showControls), useTreenodes(quals.size() > 1), quals(quals), fdata(getELogData(quals)) {
        // Here we iterate over the keys of fdata instead of the vector quals, since the keys of fdata are all
        // guarenteed to be unique
        for(const auto& qual : fdata | std::views::keys) {
            hwqData[qual] = {.tareStates = std::vector<StopWatch>(fdata.at(qual).size()), .clearState = StopWatch{}};
        }
    }

    void SensorViewer::render() {
        ImGui::PushID("SensorViewer");
        ImGui::PushID(classid);

        // Get the drawlist, and calculate the size of the plots
        ImDrawList* draw = ImGui::GetWindowDrawList();
        ImVec2 plotsize = GraphInfo::calcPlotSize();

        if(showControls) {
            if(ImGui::TimedButton("Clear All Graphs", clearAllTimer)) {
                ImGui::SameLine();
                ImGui::CircleProgressBar("##clearallprogressspinner", 7_sc, 2_sc, WHITE_COLOR,
                                         clearAllTimer.timeSince() / CONFIRM_HOLD_TIME);
                if(clearAllTimer.timeSince() > CONFIRM_HOLD_TIME) {
                    // for(const auto& qual : sensors | std::views::keys) Sensors::clearGraph(qual); // Cant yet do this
                    clearAllTimer.reset();
                }
            }

            if(ImGui::TimedButton("Tare All Devices", tareAllTimer)) {
                ImGui::SameLine();
                ImGui::CircleProgressBar("##tareallprogressspinner", 7_sc, 2_sc, WHITE_COLOR,
                                         tareAllTimer.timeSince() / CONFIRM_HOLD_TIME);
                if(tareAllTimer.timeSince() > CONFIRM_HOLD_TIME) {
                    for(const auto& [qual, channels] : fdata) {
                        HardwareChannel ch = {qual, 0};
                        for(size_t i = 0; i < channels.size(); i++, ch.channel++) {
                            if(!channels[i].times->empty()) hwctrl::requestTare(ch, calculateTare(channels[i]));
                        }
                    }
                    tareAllTimer.reset();
                }
            }

            // Not needed with hdfs
            // if(ImGui::Button("Write all to CSV")) {
            //     for(const auto& qual : sensors | std::views::keys) Sensors::writeCSV(qual);
            // }
        }

        int id = 0;
        // Iterate through each qualifier and render its data
        for(const auto& qual : quals) {
            ImGui::PushID(id++);

            if(useTreenodes) {
                if(!ImGui::TreeNode(qual.name.c_str())) {
                    ImGui::PopID();
                    continue;
                }
            }

            const auto& channels = fdata.at(qual);
            const auto& graphdata = GraphInfo::GRAPHINFO.at(qual.devclass);

            if(showControls) {
                const auto& meta = channels.at(0).times;

                // Status Square
                ImGui::Text("Sensor Status: ");
                ImGui::SameLine();
                ImVec2 pos = ImGui::GetCursorScreenPos();
                draw->AddRectFilled(pos, pos + scale(STATUS_SQUARE_SIZE), meta->empty() ? STALE_COLOR : ENABLED_COLOR);
                ImGui::Dummy(scale(STATUS_SQUARE_SIZE));
                if(ImGui::IsItemHovered()) ImGui::SetTooltip(meta->empty() ? "No data received" : "Receiving data");

                ImGui::SameLine();
                ImGui::Text(" | Data Points: %lld", meta->size());
                {
                    std::stringstream ss;
                    ss << qual.name << ": ";
                    HardwareChannel ch = {qual, 0};
                    for(size_t i = 0; i < channels.size(); i++, ch.channel++) {
                        float latestVal = channels[i].values->empty() ? 0 : channels[i].values->back();
                        if(i != 0) ss << " | ";
                        ss << renderLatestReadingsString(ch, latestVal);
                    }
                    ImGui::TextWrapped("%s", ss.str().c_str());
                }

                ImGui::Text("Tare: ");
                float percent = 0;

                for(size_t i = 0; i < channels.size(); i++) {
                    ImGui::SameLine();
                    bool lockTare = channels[i].times->empty();
                    if(lockTare) ImGui::BeginDisabled();
                    auto& timer = hwqData[qual].tareStates[i];
                    if(ImGui::TimedButton(graphdata.lines[i].legend.c_str(), timer)) {
                        percent = timer.timeSince() / CONFIRM_HOLD_TIME;
                        if(percent >= 1.0f) {
                            HardwareChannel temp = {qual, static_cast<uint8_t>(i)};
                            hwctrl::requestTare(temp, calculateTare(channels[i]));
                            timer.reset();
                        }
                    }
                    if(lockTare) ImGui::EndDisabled();
                }

                ImGui::SameLine();
                ImGui::CircleProgressBar("##tareprogressspinner", 7_sc, 2_sc, WHITE_COLOR, percent);

                // TODO: Add back ability to clear graph
            }

            for(size_t i = 0; i < graphdata.axes.size(); i++) {
                if(!ImPlot::BeginPlot(graphdata.axes[i].independentGraphName.c_str(), plotsize)) continue;
                ImPlot::SetupAxes("Time (s)", graphdata.axes[i].name.c_str(), ImPlotAxisFlags_AutoFit,
                                  ImPlotAxisFlags_AutoFit);

                bool autofit =
                    ImPlot::GetCurrentContext()->CurrentPlot->Axes[ImAxis_X1].Flags & ImPlotAxisFlags_AutoFit;

                for(size_t j = 0; j < graphdata.lines.size(); j++) {
                    // This nested loop wont actually be that bad since the lines array will (at the time of writing)
                    // usually have only 1 thing, and a max of 4 items
                    if(graphdata.lines[j].axis != i) continue;

                    TargetFloatPlotData pd;
                    if(autofit) pd = TargetFloatPlotData(&channels[j], NUM_GRAPH_POINTS);
                    else {
                        auto plotDims = ImPlot::GetPlotLimits();
                        auto min = static_cast<uint32_t>(std::max(0., plotDims.X.Min));
                        auto max = static_cast<uint32_t>(std::max(0., plotDims.X.Max));
                        pd = TargetFloatPlotData(&channels[j], NUM_GRAPH_POINTS, min, max);
                    }

                    ImPlot::PlotLineG(graphdata.lines[j].legend.c_str(), implotTargetFloat, &pd, pd.numPoints);
                }

                ImPlot::EndPlot();
            }

            if(useTreenodes) {
                ImGui::Separator();
                ImGui::TreePop();
            }

            ImGui::PopID();
        }

        ImGui::PopID();
        ImGui::PopID();
    }
} // namespace LRI::RCI
