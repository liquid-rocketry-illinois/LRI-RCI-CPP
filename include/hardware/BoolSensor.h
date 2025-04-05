#ifndef BOOLSENSOR_H
#define BOOLSENSOR_H

#include <map>
#include <set>

#include "utils.h"
#include "HardwareQualifier.h"

namespace LRI::RCI {
    class BoolSensors {
        static constexpr int REFRESH_TIME = 5;

        struct BoolSensorState;

        std::map<HardwareQualifier, BoolSensorState*> state;
        StopWatch refreshTimer;

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
        void update();
    };
}

#endif //BOOLSENSOR_H
