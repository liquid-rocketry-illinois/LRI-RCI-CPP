#ifndef ANGLEDACTUATOR_H
#define ANGLEDACTUATOR_H

#include <set>
#include <vector>

#include "HardwareQualifier.h"
#include "Sensors.h"

namespace LRI::RCI::AngledActuators {
    void reset();
    void setHardwareConfig(std::set<HardwareQualifier>& quals);
    void setActuatorPos(const HardwareQualifier& qual, float degrees);
    [[nodiscard]] const std::vector<Sensors::DataPoint>* getState(const HardwareQualifier& qual);
    void refreshAll();
} // namespace LRI::RCI::AngledActuators

#endif // ANGLEDACTUATOR_H
