#ifndef ANGLEDACTUATOR_H
#define ANGLEDACTUATOR_H

#include <set>
#include <tuple>
#include <vector>

#include "HardwareQualifier.h"
#include "hardware/EventLog.h"

namespace LRI::RCI::AngledActuators {
    struct AngledActuatorState {
        float value;
        bool stale;
    };

    void setHardwareConfig(const std::set<HardwareQualifier>& quals);

    [[nodiscard]] const AngledActuatorState* getLatestState(const HardwareChannel& qual);
    [[nodiscard]] FloatData getFullLog(const HardwareChannel& qual);

    void setActuatorPos(const HardwareQualifier& qual, float degrees);

    void reset();
    void refreshAll();

    RCP_Error receiveRCPUpdate(const RCP_1F& f1);
} // namespace LRI::RCI::AngledActuators

#endif // ANGLEDACTUATOR_H
