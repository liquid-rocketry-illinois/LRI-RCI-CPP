#ifndef STEPPERVIEWER_H
#define STEPPERVIEWER_H

#include <map>
#include <set>
#include <vector>

#include "RCP_Host/RCP_Host.h"
#include "WModule.h"
#include "hardware/HardwareQualifier.h"
#include "hardware/Steppers.h"

namespace LRI::RCI {
    // A window for showing and controlling states of stepper motors
    class StepperViewer : public WModule {
        // Maps control modes to UI button names
        static constexpr std::map<RCP_StepperControlMode_t, std::vector<const char*>> BTN_NAMES{
            {RCP_STEPPER_ABSOLUTE_POS_CONTROL, {"Absolute Positioning##", " degrees###input"}},
            {RCP_STEPPER_RELATIVE_POS_CONTROL, {"Relative Positioning##", " degrees###input"}},
            {RCP_STEPPER_SPEED_CONTROL, {"Velocity Control##", " degrees/s###input"}},
        };

        static int CLASSID;

        struct Input {
            float val;
            RCP_StepperControlMode_t mode;
        };

        const int classid;
        const bool refreshButton;
        // Maps stepper IDs to their structure
        std::map<HardwareQualifier, const Steppers::Stepper*> steppers;
        std::map<HardwareQualifier, Input> inputs;

    public:
        explicit StepperViewer(const std::set<HardwareQualifier>&& quals, bool refreshButton = false);
        ~StepperViewer() override = default;

        // Overridden render function
        void render() override;
    };
}

#endif //STEPPERVIEWER_H
