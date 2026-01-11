#ifndef HARDWARE_SOLENOIDS_H
#define HARDWARE_SOLENOIDS_H

#include <set>
#include <tuple>

#include "HardwareQualifier.h"
#include "RCP_Host/RCP_Host.h"
#include "hardware/EventLog.h"

// Singleton for representing the state of all simple actuators
namespace LRI::RCI::SimpleActuators {
    struct SimpleActuatorState {
        bool state;
        bool stale;
    };

    // Sets which qualifiers are active solenoids
    void setHardwareConfig(const std::set<HardwareQualifier>& solIds);

    // Get pointer that viewer classes can use to track an actuator
    [[nodiscard]] const SimpleActuatorState* getLatestState(const HardwareChannel& qual);
    [[nodiscard]] BoolData getFullLog(const HardwareChannel& ch);

    // Request a write to an actuator
    void setActuatorState(const HardwareQualifier& qual, RCP_SimpleActuatorState newState);

    // Reset class state
    void reset();

    // Request refresh of all actuators
    void refreshAll();

    // Receive updates from RCP
    RCP_Error receiveRCPUpdate(RCP_SimpleActuatorData data);
} // namespace LRI::RCI::SimpleActuators

#endif // HARDWARE_SOLENOIDS_H
