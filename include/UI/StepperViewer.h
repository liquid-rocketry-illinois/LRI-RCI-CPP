#ifndef STEPPERCONTROL_H
#define STEPPERCONTROL_H

#include <map>
#include <string>
#include <vector>
#include <RCP_Host/RCP_Host.h>

#include "BaseUI.h"
#include "hardware/HardwareQualifier.h"
#include "hardware/Steppers.h"

namespace LRI::RCI {
    // A window for showing and controlling states of stepper motors
    class StepperViewer : public BaseUI {
        // Maps control modes to UI button names
        static const std::map<uint8_t, std::vector<std::string>> BTN_NAMES; // Unfortunately, it can't be constexpr

        struct Input {
            float val;
            RCP_StepperControlMode_t mode;
        };

        const bool refreshButton;
        // Maps stepper IDs to their structure
        std::map<HardwareQualifier, const Steppers::Stepper*> steppers;
        std::map<HardwareQualifier, Input> inputs;

    public:
        StepperViewer(const std::set<HardwareQualifier>&& quals, bool refreshButton);

        // Overridden render function
        void render() override;

        // Custom reset
        void reset() override;

        ~StepperViewer() override = default;
    };
}

#endif //STEPPERCONTROL_H
