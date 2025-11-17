#ifndef BOOLSENSOR_H
#define BOOLSENSOR_H

#include <map>
#include <set>

#include "HardwareQualifier.h"
#include "utils.h"

// Hardware singleton for the BoolSensors
namespace LRI::RCI::BoolSensors {
    struct BoolSensorState {
        bool open = false;
        bool stale = true;
    };

    // Receive updates from RCP
    int receiveRCPUpdate(RCP_BoolData data);

    // Set which qualifiers are tracked by the singleton, and the interval for refreshes
    void setHardwareConfig(const std::set<HardwareQualifier>& ids, int _refreshTime = 5);

    // Clears storage and resets class to defaults
    void reset();

    // Gets a pointer that can be tracked and stored by viewer classes
    [[nodiscard]] const BoolSensorState* getState(const HardwareQualifier& qual);

    // Request a refresh of all tracked qualifiers
    void refreshAll();

    // Called from main to check timer and refresh state
    void update();
} // namespace LRI::RCI

#endif // BOOLSENSOR_H
