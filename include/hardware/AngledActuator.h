#ifndef ANGLEDACTUATOR_H
#define ANGLEDACTUATOR_H

#include <set>
#include <map>
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

    void setHardwareConfig(std::set<HardwareQualifier>& quals);
    void setActuatorPos(const HardwareQualifier& qual, float degrees);
    [[nodiscard]] const std::vector<Sensors::DataPoint>* getState(const HardwareQualifier& qual);
  };
}

#endif //ANGLEDACTUATOR_H
