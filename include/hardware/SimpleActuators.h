#ifndef HARDWARE_SOLENOIDS_H
#define HARDWARE_SOLENOIDS_H

#include <map>
#include <set>

#include "RCP_Host/RCP_Host.h"
#include "HardwareQualifier.h"

// Singleton for representing the state of all simple actuators
namespace LRI::RCI {
    class SimpleActuators {
        struct ActuatorState;

        // Maps qualifiers to state pointers
        std::map<HardwareQualifier, ActuatorState*> state;

        SimpleActuators() = default;
        ~SimpleActuators();

    public:
        struct ActuatorState {
            bool open;
            bool stale;
        };

        // Get singleton instance
        static SimpleActuators* getInstance();

        // Receive updates from RCP
        void receiveRCPUpdate(const HardwareQualifier& qual, bool newState);

        // Sets which qualifiers are active solenoids
        void setHardwareConfig(const std::set<HardwareQualifier>& solIds);

        // Reset class state
        void reset();

        // Get pointer that viewer classes can use to track an actuator
        [[nodiscard]] const ActuatorState* getState(const HardwareQualifier& qual) const;

        // Request refresh of all actuators
        void refreshAll() const;

        // Request a write to an actuator
        void setActuatorState(const HardwareQualifier& qual, RCP_SimpleActuatorState newState);
    };
}


#endif // HARDWARE_SOLENOIDS_H
