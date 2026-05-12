#ifndef LRI_CONTROL_PANEL_MOTORS_H
#define LRI_CONTROL_PANEL_MOTORS_H

#include <set>
#include <vector>

#include "HardwareQualifier.h"
#include "Sensors.h"

namespace LRI::RCI::Motors {
    void reset();
    void setHarwareConfig(const std::set<HardwareQualifier>& quals);
    void setState(const HardwareQualifier& qual, float value);
    [[nodiscard]] const std::vector<Sensors::DataPoint>* getState(const HardwareQualifier& qual);
    void refreshAll();
} // namespace LRI::RCI::Motors

#endif // LRI_CONTROL_PANEL_MOTORS_H
