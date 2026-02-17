#ifndef LRI_CONTROL_PANEL_MOTORS_H
#define LRI_CONTROL_PANEL_MOTORS_H

#include <set>

#include "HardwareQualifier.h"

namespace LRI::RCI::Motors {
    struct Motor {
        float value;
        bool stale;
    };

    int receiveRCPUpdate(const HardwareQualifier& qual, const float& data);

    void setHarwareConfig(const std::set<HardwareQualifier>& quals);

    void reset();

    void refreshAll();

    [[nodiscard]] const Motor* getState(const HardwareQualifier& qual);

    void setState(const HardwareQualifier& qual, float value);

} // namespace LRI::RCI::Motors

#endif // LRI_CONTROL_PANEL_MOTORS_H
