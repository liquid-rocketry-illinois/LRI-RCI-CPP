#ifndef BOOLSENSORVIEWER_H
#define BOOLSENSORVIEWER_H

#include <map>
#include <set>

#include "Windowlet.h"
#include "hardware/EventLog.h"
#include "hardware/HardwareQualifier.h"

// Viewer class for Bool Sensors
namespace LRI::RCI {
    class BoolSensorViewer : public WModule {
        struct SData {
            EventLog::TargetUint data;
            bool stale;
        };
        // If a refresh button should be rendered at the top of this module
        const bool refreshButton;

        // Which sensors are being tracked and a pointer to their current state
        std::map<HardwareChannel, SData> sensors;

    public:
        // Constructor that takes in which sensors to track and if a refresh button should be rendered
        explicit BoolSensorViewer(const std::set<HardwareChannel>& quals, bool refreshButton = false);

        // Rendering
        void render() override;

        ~BoolSensorViewer() override = default;
    };
} // namespace LRI::RCI

#endif // BOOLSENSORVIEWER_H
