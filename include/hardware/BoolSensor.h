#ifndef BOOLSENSOR_H
#define BOOLSENSOR_H

#include <set>
#include <tuple>

#include "EventLog.h"
#include "HardwareQualifier.h"

// Hardware singleton for the BoolSensors
namespace LRI::RCI::BoolSensors {
    struct BoolSensorState {
        bool state;
        bool stale;
    };

    // Set which qualifiers are tracked by the singleton, and the interval for refreshes
    void setHardwareConfig(const std::set<HardwareQualifier>& ids, int _refreshTime = 5);

    // Gets a pointer that can be tracked and stored by viewer classes
    [[nodiscard]] const BoolSensorState* getLatestState(const HardwareChannel& qual);
    [[nodiscard]] BoolData getFullLog(const HardwareChannel& qual);

    // Clears storage and resets class to defaults
    void reset();

    // Request a refresh of all tracked qualifiers
    void refreshAll();

    // Receive updates from RCP
    RCP_Error receiveRCPUpdate(RCP_BoolData data);

    // Called from main to check timer and refresh state
    void update();
} // namespace LRI::RCI

#endif // BOOLSENSOR_H
