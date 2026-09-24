#ifndef LRI_CONTROL_PANEL_ABRIDGEDSENSORVIEWER_H
#define LRI_CONTROL_PANEL_ABRIDGEDSENSORVIEWER_H

#include <map>
#include <vector>

#include "Windowlet.h"
#include "hardware/HardwareQualifier.h"
#include "hardware/EventLog.h"

namespace LRI::RCI {
    class AbridgedSensorViewer : public WModule {
        std::map<HardwareChannel, EventLog::TargetFloat> data;
        std::vector<std::vector<HardwareChannel>> sensors;

    public:
        explicit AbridgedSensorViewer(const std::vector<std::vector<HardwareChannel>>& sensors);
        ~AbridgedSensorViewer() override = default;

        void render() override;
    };
}

#endif // LRI_CONTROL_PANEL_ABRIDGEDSENSORVIEWER_H
