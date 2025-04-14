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
        static const std::map<RCP_StepperControlMode, std::vector<const char*>> BTN_NAMES;

        static int CLASSID;

        struct Input {
            float val;
            RCP_StepperControlMode mode;
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
