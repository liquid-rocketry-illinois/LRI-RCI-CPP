#ifndef STEPPERS_H
#define STEPPERS_H

#include <map>
#include <set>

#include "HardwareQualifier.h"
#include "RCP_Host/RCP_Host.h"

// Singleton for all stepper motors
namespace LRI::RCI {
    class Steppers {
    public:
        struct Stepper {
            float position;
            float speed;
            bool stale;
        };

    private:

        // Storage container for steppers
        std::map<HardwareQualifier, Stepper> motors;

        Steppers() = default;
        ~Steppers();

    public:
        // Get singleton instance
        static Steppers* getInstance();

        // Receive updates from RCP
        int receiveRCPUpdate(const HardwareQualifier& qual, const float& pos, const float& speed);

        // Set which qualifiers are in use
        void setHardwareConfig(const std::set<HardwareQualifier>& motorlist);

        // Reset class state
        void reset();

        // Request refresh of all steppers
        void refreshAll();

        // Get pointer viewer classes can use to track a stepper
        [[nodiscard]] const Stepper* getState(const HardwareQualifier& qual) const;

        // Request a write to a stepper
        void setState(const HardwareQualifier& qual, RCP_StepperControlMode controlMode, float value);
    };
} // namespace LRI::RCI

#endif // STEPPERS_H
