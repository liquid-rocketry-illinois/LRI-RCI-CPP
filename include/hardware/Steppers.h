#ifndef STEPPERS_H
#define STEPPERS_H

#include <string>
#include <map>
#include <vector>

#include "RCP_Host/RCP_Host.h"
#include "HardwareQualifier.h"

namespace LRI::RCI {
    class Steppers {
        struct Stepper;
        static Steppers* instance;

        std::map<HardwareQualifier, Stepper> motors;

        Steppers() = default;

    public:
        struct Stepper {
            RCP_StepperControlMode_t controlMode = RCP_STEPPER_ABSOLUTE_POS_CONTROL;
            float position{};
            float speed{};
            bool stale{};
            float controlVal{};
        };

        static Steppers* getInstance();

        void receiveRCPUpdate(const HardwareQualifier& qual, const float& pos, const float& speed);
        void setHardwareConfig(const std::vector<HardwareQualifier>& motorlist);

        void refreshAll() const;
        [[nodiscard]] const std::map<HardwareQualifier, Stepper>& getState() const;
        void setState(const HardwareQualifier& qual, RCP_StepperControlMode_t controlMode, float value);
    };
}

#endif //STEPPERS_H
