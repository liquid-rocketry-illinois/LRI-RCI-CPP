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
        struct HWQData {
            std::vector<StopWatch> tareStates;
            StopWatch clearState;
            // std::vector<bool> autoFits;
        };

        const bool showControls;
        const bool useTreenodes;

        static std::map<HardwareQualifier, std::vector<EventLog::TargetFloat>>
        getELogData(const std::vector<HardwareQualifier>& quals);

        const std::vector<HardwareQualifier> quals;
        const std::map<HardwareQualifier, std::vector<EventLog::TargetFloat>> fdata;

        // The tare and clear states for each sensor
        std::map<HardwareQualifier, HWQData> hwqData;
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
