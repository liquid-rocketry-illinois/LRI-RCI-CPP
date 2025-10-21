#ifndef HARDWARE_SOLENOIDS_H
#define HARDWARE_SOLENOIDS_H

#include <map>
#include <set>

#include "HardwareQualifier.h"
#include "RCP_Host/RCP_Host.h"

// Singleton for representing the state of all simple actuators
namespace LRI::RCI {
    class SimpleActuators {
    public:
        struct ActuatorState {
            bool open;
            bool stale;
        };

    private:
        // Maps qualifiers to state pointers
        std::map<HardwareQualifier, ActuatorState> state;

        SimpleActuators() = default;
        ~SimpleActuators();

    public:
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
        void refreshAll();

        // Request a write to an actuator
        void setActuatorState(const HardwareQualifier& qual, RCP_SimpleActuatorState newState);
    };
} // namespace LRI::RCI


#endif // HARDWARE_SOLENOIDS_H
