#ifndef STEPPERS_H
#define STEPPERS_H

#include <set>

#include "HardwareQualifier.h"
#include "RCP_Host/RCP_Host.h"

// Singleton for all stepper motors
namespace LRI::RCI::Steppers {
    struct Stepper {
        float position;
        float speed;
        bool stale;
    };

    // Receive updates from RCP
    int receiveRCPUpdate(const HardwareQualifier& qual, const float& pos, const float& speed);

    // Set which qualifiers are in use
    void setHardwareConfig(const std::set<HardwareQualifier>& motorlist);

    // Reset class state
    void reset();

    // Request refresh of all steppers
    void refreshAll();

    // Get pointer viewer classes can use to track a stepper
    [[nodiscard]] const Stepper* getState(const HardwareQualifier& qual);

    // Request a write to a stepper
    void setState(const HardwareQualifier& qual, RCP_StepperControlMode controlMode, float value);
} // namespace LRI::RCI::Steppers

#endif // STEPPERS_H
