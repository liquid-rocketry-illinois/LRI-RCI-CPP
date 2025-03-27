#ifndef STEPPERVIEWER_H
#define STEPPERVIEWER_H

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
        static constexpr std::map<RCP_StepperControlMode_t, std::vector<std::string>> BTN_NAMES{
            {RCP_STEPPER_ABSOLUTE_POS_CONTROL, {"Absolute Positioning##", "degrees"}},
            {RCP_STEPPER_RELATIVE_POS_CONTROL, {"Relative Positioning##", "degrees"}},
            {RCP_STEPPER_SPEED_CONTROL, {"Velocity Control##", "degrees/s"}},
        };

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

#endif //STEPPERVIEWER_H
