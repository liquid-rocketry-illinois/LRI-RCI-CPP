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
    // A window module for showing and controlling states of stepper motors
    class StepperViewer : public WModule {
        // Maps control modes to UI button names
        static const std::map<RCP_StepperControlMode, std::vector<const char*>> BTN_NAMES;

        static int CLASSID;
        const int classid;

        struct Input {
            float val;
            RCP_StepperControlMode mode;
        };

        // If a refresh button should be shown at the top
        const bool refreshButton;

        // Maps stepper IDs to structure pointers which are updated by Steppers
        std::map<HardwareQualifier, const Steppers::Stepper*> steppers;

        // For storing which input mode each stepper is in
        std::map<HardwareQualifier, Input> inputs;

    public:
        explicit StepperViewer(const std::set<HardwareQualifier>&& quals, bool refreshButton = false);
        ~StepperViewer() override = default;

        // Overridden render function
        void render() override;
    };
}

#endif //STEPPERVIEWER_H
