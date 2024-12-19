#ifndef STEPPERCONTROL_H
#define STEPPERCONTROL_H

#include <map>
#include <string>
#include <vector>
#include <RCP_Host/RCP_Host.h>

#include "BaseUI.h"

namespace LRI::RCI {
    class Steppers : public BaseUI {
        static const std::map<uint8_t, std::vector<std::string>> BTN_NAMES; // Unfortunately, it can't be constexpr

        struct Stepper {
            RCP_StepperControlMode_t controlMode;
            int32_t position;
            int32_t speed;
            bool stale;
            std::string name;
            float controlVal;
        };

        static Steppers* instance;

        Steppers() = default;
        std::map<uint8_t, Stepper> steppers;

    public:
        static Steppers* getInstance();

        void render() override;
        void setHardwareConfig(const std::map<uint8_t, std::string>& ids);
        void receiveRCPUpdate(const RCP_StepperData& state);

        ~Steppers() override = default;
    };
}

#endif //STEPPERCONTROL_H
