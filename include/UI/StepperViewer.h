#ifndef STEPPERVIEWER_H
#define STEPPERVIEWER_H

#include <map>
#include <set>
#include <vector>

#include "RCP_Host/RCP_Host.h"
#include "Windowlet.h"
#include "hardware/HardwareQualifier.h"
#include "hardware/EventLog.h"

namespace LRI::RCI {
    // A window module for showing and controlling states of stepper motors
    class StepperViewer : public WModule {
        // Maps control modes to UI button names
        static const std::map<RCP_StepperControlMode, std::vector<const char*>> BTN_NAMES;

        struct SData {
            std::vector<EventLog::TargetFloat> data;
            float val = 0;
            RCP_StepperControlMode mode = RCP_STEPPER_ABSOLUTE_POS_CONTROL;
        };

        // If a refresh button should be shown at the top
        const bool refreshButton;

        // Maps stepper IDs to structure pointers which are updated by Steppers
        std::map<HardwareQualifier, SData> steppers;

    public:
        explicit StepperViewer(const std::set<HardwareQualifier>& quals, bool refreshButton = false);
        ~StepperViewer() override = default;

        // Overridden render function
        void render() override;
    };
} // namespace LRI::RCI

#endif // STEPPERVIEWER_H
