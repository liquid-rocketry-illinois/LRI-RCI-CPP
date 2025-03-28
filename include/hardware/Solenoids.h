#ifndef HARDWARE_SOLENOIDS_H
#define HARDWARE_SOLENOIDS_H

#include <map>
#include <set>

#include "RCP_Host/RCP_Host.h"
#include "HardwareQualifier.h"

namespace LRI::RCI {
    class Solenoids {
        struct SolenoidState;

        std::map<HardwareQualifier, SolenoidState*> state;

        Solenoids() = default;
        ~Solenoids();

    public:
        struct SolenoidState {
            bool open;
            bool stale;
        };

        static Solenoids* getInstance();
        void receiveRCPUpdate(const HardwareQualifier& qual, bool newState);
        void setHardwareConfig(const std::set<HardwareQualifier>& solIds);
        void reset();

        [[nodiscard]] const SolenoidState* getState(const HardwareQualifier& qual) const;
        void refreshAll() const;
        void setSolenoidState(const HardwareQualifier& qual, RCP_SolenoidState_t newState);
    };
}


#endif // HARDWARE_SOLENOIDS_H
