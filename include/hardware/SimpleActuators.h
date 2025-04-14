#ifndef HARDWARE_SOLENOIDS_H
#define HARDWARE_SOLENOIDS_H

#include <map>
#include <set>

#include "RCP_Host/RCP_Host.h"
#include "HardwareQualifier.h"

namespace LRI::RCI {
    class SimpleActuators {
        struct ActuatorState;

        std::map<HardwareQualifier, ActuatorState*> state;

        SimpleActuators() = default;
        ~SimpleActuators();

    public:
        struct ActuatorState {
            bool open;
            bool stale;
        };

        static SimpleActuators* getInstance();
        void receiveRCPUpdate(const HardwareQualifier& qual, bool newState);
        void setHardwareConfig(const std::set<HardwareQualifier>& solIds);
        void reset();

        [[nodiscard]] const ActuatorState* getState(const HardwareQualifier& qual) const;
        void refreshAll() const;
        void setActuatorState(const HardwareQualifier& qual, RCP_SimpleActuatorState newState);
    };
}


#endif // HARDWARE_SOLENOIDS_H
