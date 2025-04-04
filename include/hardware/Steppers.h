#ifndef STEPPERS_H
#define STEPPERS_H

#include <map>
#include <set>

#include "RCP_Host/RCP_Host.h"
#include "HardwareQualifier.h"

namespace LRI::RCI {
    class Steppers {
        struct Stepper;

        std::map<HardwareQualifier, Stepper*> motors;

        Steppers() = default;
        ~Steppers();

    public:
        struct Stepper {
            float position{};
            float speed{};
            bool stale{};
        };

        static Steppers* getInstance();

        void receiveRCPUpdate(const HardwareQualifier& qual, const float& pos, const float& speed);
        void setHardwareConfig(const std::set<HardwareQualifier>& motorlist);
        void reset();

        void refreshAll() const;
        [[nodiscard]] const Stepper* getState(const HardwareQualifier& qual) const;
        void setState(const HardwareQualifier& qual, RCP_StepperControlMode controlMode, float value);
    };
}

#endif //STEPPERS_H
