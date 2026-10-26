#ifndef LRI_CONTROL_PANEL_MULTISENSORVIEWER_H
#define LRI_CONTROL_PANEL_MULTISENSORVIEWER_H

#include <map>
#include <string>
#include <vector>

#include "Windowlet.h"
#include "hardware/HardwareQualifier.h"
#include "hardware/EventLog.h"

namespace LRI::RCI {
    class MultiSensorViewer : public WModule {
    public:
        struct GraphData {
            std::string title;
            std::vector<HardwareChannel> channels;
        };

    private:
        struct ChannelData {
            EventLog::TargetFloat data;
            std::string lineName;
        };

        struct GraphMeta {
            std::string title;
            std::string axis;
            std::vector<HardwareChannel> channels;
        };

        static std::vector<GraphMeta> makeGraphMeta(const std::vector<GraphData>& graphs);
        static std::map<HardwareChannel, ChannelData> makeChannelData(const std::vector<GraphData>& graphs);

        const std::vector<GraphMeta> graphs;
        const std::map<HardwareChannel, ChannelData> channelData;

    public:
        explicit MultiSensorViewer(const std::vector<GraphData>& graphs);
        ~MultiSensorViewer() override = default;

        // Overridden render function
        void render() override;
    };
} // namespace LRI::RCI

#endif // LRI_CONTROL_PANEL_MULTISENSORVIEWER_H
