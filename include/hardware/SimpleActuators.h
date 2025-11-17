#ifndef HARDWARE_SOLENOIDS_H
#define HARDWARE_SOLENOIDS_H

#include <map>
#include <set>

#include "HardwareQualifier.h"
#include "RCP_Host/RCP_Host.h"

// Singleton for representing the state of all simple actuators
namespace LRI::RCI::SimpleActuators {
    struct ActuatorState {
        bool open;
        bool stale;
    };

    // Receive updates from RCP
    int receiveRCPUpdate(RCP_SimpleActuatorData data);

    // Sets which qualifiers are active solenoids
    void setHardwareConfig(const std::set<HardwareQualifier>& solIds);

    // Reset class state
    void reset();

    // Get pointer that viewer classes can use to track an actuator
    [[nodiscard]] const ActuatorState* getState(const HardwareQualifier& qual);

    // Request refresh of all actuators
    void refreshAll();

    // Request a write to an actuator
    void setActuatorState(const HardwareQualifier& qual, RCP_SimpleActuatorState newState);
} // namespace LRI::RCI::SimpleActuators


#endif // HARDWARE_SOLENOIDS_H
