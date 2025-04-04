#ifndef BOOLSENSOR_H
#define BOOLSENSOR_H

#include <map>
#include <set>

#include "RCP_Host/RCP_Host.h"

#include "HardwareQualifier.h"

namespace LRI::RCI {
    class BoolSensors {
        struct BoolSensorState;

        std::map<HardwareQualifier, BoolSensorState*> state;

        BoolSensors() = default;
        ~BoolSensors();

    public:
        struct BoolSensorState {
            bool open;
            bool stale;
        };

        static BoolSensors* getInstance();

        void receiveRCPUpdate(const HardwareQualifier& qual, bool newstate);
        void setHardwareConfig(const std::set<HardwareQualifier>& ids);
        void reset();
        [[nodiscard]] const BoolSensorState* getState(const HardwareQualifier& qual) const;
        void refreshAll() const;


    };
}

#endif //BOOLSENSOR_H
