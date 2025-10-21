#ifndef ANGLEDACTUATOR_H
#define ANGLEDACTUATOR_H

#include <map>
#include <set>
#include <vector>

#include "HardwareQualifier.h"
#include "Sensors.h"

namespace LRI::RCI {
    class AngledActuators {
        std::set<HardwareQualifier> actuators;

        AngledActuators() = default;
        ~AngledActuators() = default;

    public:
        static AngledActuators* getInstance();

        void reset();
        void setHardwareConfig(std::set<HardwareQualifier>& quals);
        void setActuatorPos(const HardwareQualifier& qual, float degrees);
        [[nodiscard]] const std::vector<Sensors::DataPoint>* getState(const HardwareQualifier& qual) const;
        void refreshAll();
    };
} // namespace LRI::RCI

#endif // ANGLEDACTUATOR_H
