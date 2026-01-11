#ifndef STEPPERS_H
#define STEPPERS_H

#include <set>

#include "RCP_Host/RCP_Host.h"

#include "HardwareQualifier.h"
#include "hardware/EventLog.h"

// Singleton for all stepper motors
namespace LRI::RCI::Steppers {
    struct Stepper {
        float position;
        float speed;
        bool stale;
    };

    // Set which qualifiers are in use
    void setHardwareConfig(const std::set<HardwareQualifier>& motorlist);

    // Get pointer viewer classes can use to track a stepper
    [[nodiscard]] const Stepper* getLatestState(const HardwareQualifier& qual);
    [[nodiscard]] FloatData getFullLog(const HardwareChannel& ch);

    // Request a write to a stepper
    void setState(const HardwareQualifier& qual, RCP_StepperControlMode controlMode, float value);

    // Reset class state
    void reset();

    // Request refresh of all steppers
    void refreshAll();

    // Receive updates from RCP
    RCP_Error receiveRCPUpdate(const RCP_2F& data);
} // namespace LRI::RCI::Steppers

#endif // STEPPERS_H
