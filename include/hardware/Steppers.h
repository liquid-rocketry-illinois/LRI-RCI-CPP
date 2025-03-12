#ifndef STEPPERS_H
#define STEPPERS_H

#include <map>
#include <vector>

#include "RCP_Host/RCP_Host.h"
#include "HardwareQualifier.h"

namespace LRI::RCI {
    class Steppers {
        struct Stepper;

        std::map<HardwareQualifier, Stepper*> motors;

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
        void reset();

        void refreshAll() const;
        [[nodiscard]] const std::map<HardwareQualifier, Stepper*>* getState() const;
        [[nodiscard]] const Stepper* getState(const HardwareQualifier& qual) const;
        void setState(const HardwareQualifier& qual, RCP_StepperControlMode_t controlMode, float value);
    };
}

#endif //STEPPERS_H
