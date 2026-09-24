#ifndef SENSORVIEWER_H
#define SENSORVIEWER_H

#include <map>
#include <vector>

#include "Windowlet.h"
#include "hardware/EventLog.h"
#include "hardware/HardwareQualifier.h"

namespace LRI::RCI {
    // A window module which shows graphs logging received sensor datapoints, or an abridged
    // sensor output which is just the sensor values as text
    class SensorViewer : public WModule {
        const bool showControls;
        const bool useTreenodes;

        static std::map<HardwareQualifier, std::vector<EventLog::TargetFloat>>
        getELogData(const std::vector<HardwareQualifier>& quals);

        const std::vector<HardwareQualifier> quals;
        const std::map<HardwareQualifier, std::vector<EventLog::TargetFloat>> fdata;

        // Holds the data vector pointers mapped to their qualifiers. This pointer is updated by the Sensors class
        // std::vector<std::pair<HardwareQualifier, const std::vector<Sensors::DataPoint>*>> sensors;

        // Render all the graphs for a particular qualifier
        // static void renderGraphs(const HardwareQualifier& qual, const std::vector<Sensors::DataPoint>* data,
        //                          const ImVec2& plotsize);

        // The tare and clear states for each sensor
        std::map<HardwareQualifier, std::vector<StopWatch>> tarestate;
        std::map<HardwareQualifier, StopWatch> clearState;
        StopWatch tareAllTimer;
        StopWatch clearAllTimer;

    public:
        explicit SensorViewer(const std::vector<HardwareQualifier>& quals, bool showControls = false);
        ~SensorViewer() override = default;

        // Overridden render function
        void render() override;
    };
} // namespace LRI::RCI

#endif // SENSORVIEWER_H
