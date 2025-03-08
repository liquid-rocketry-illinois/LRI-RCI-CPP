#ifndef HARDWARE_SOLENOIDS_H
#define HARDWARE_SOLENOIDS_H

#include <map>
#include <vector>

#include "RCP_Host/RCP_Host.h"
#include "HardwareQualifier.h"

namespace LRI::RCI {
    class Solenoids {
        static Solenoids* instance;
        struct SolenoidState;

        std::map<HardwareQualifier, SolenoidState> state;

        Solenoids() = default;

    public:
        struct SolenoidState {
            bool open;
            bool fresh;
        };

        static Solenoids* getInstance();
        void receiveRCPUpdate(const HardwareQualifier& qual, bool newState);
        void setHardwareConfig(const std::vector<HardwareQualifier>& solIds);

        [[nodiscard]] const std::map<HardwareQualifier, SolenoidState>* getState() const;
        void setSolenoidState(const HardwareQualifier& qual, RCP_SolenoidState_t newState);
    };
}


#endif // HARDWARE_SOLENOIDS_H
