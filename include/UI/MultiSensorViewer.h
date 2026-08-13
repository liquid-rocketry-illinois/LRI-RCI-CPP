#ifndef LRI_CONTROL_PANEL_MULTISENSORVIEWER_H
#define LRI_CONTROL_PANEL_MULTISENSORVIEWER_H

#include <map>
#include <string>
#include <vector>

#include "Windowlet.h"
#include "hardware/HardwareQualifier.h"
#include "hardware/Sensors.h"

namespace LRI::RCI {
    class MultiSensorViewer : public WModule {
    public:
        struct GraphData {
            std::string title;
            std::vector<HardwareChannel> channels;
        };

    private:
        static std::map<HardwareQualifier, const std::vector<Sensors::DataPoint>*>
        extractSensors(const std::vector<GraphData>& graphs);
        static std::map<HardwareChannel, std::string> extractLineNames(const std::vector<GraphData>& graphs);
        static std::vector<std::string> extractTitles(const std::vector<GraphData>& graphs);
        static std::vector<std::string> extractAxistList(const std::vector<GraphData>& graphs);
        static std::vector<std::vector<HardwareChannel>> extractChannels(const std::vector<GraphData>& graphs);

        const std::map<HardwareQualifier, const std::vector<Sensors::DataPoint>*> data;
        const std::map<HardwareChannel, std::string> lineNames;
        const std::vector<std::string> titles;
        const std::vector<std::string> axislist;
        const std::vector<std::vector<HardwareChannel>> channels;

        // The tare and clear states for each sensor
        std::map<HardwareQualifier, StopWatch[4]> tarestate;
        std::map<HardwareQualifier, StopWatch> clearState;
        StopWatch tareAllTimer;
        StopWatch clearAllTimer;

    public:
        explicit MultiSensorViewer(const std::vector<GraphData>& graphs);
        ~MultiSensorViewer() override = default;

        // Overridden render function
        void render() override;
    };
} // namespace LRI::RCI

#endif // LRI_CONTROL_PANEL_MULTISENSORVIEWER_H
