#ifndef STEPPERCONTROL_H
#define STEPPERCONTROL_H

#include <map>
#include <string>
#include <vector>
#include <RCP_Host/RCP_Host.h>

#include "BaseUI.h"

namespace LRI::RCI {
    // A window for showing and controlling states of stepper motors
    class StepperViewer : public BaseUI {
        // Maps control modes to UI button names
        static const std::map<uint8_t, std::vector<std::string>> BTN_NAMES; // Unfortunately, it can't be constexpr

        // Structure representing a stepper motor. Contains the current UI selected control mode, the current actual
        // position and speed, whether the current data is stale, a user defined name, and the current inputted value
        // from the user
        struct Stepper {
            RCP_StepperControlMode_t controlMode;
            float position;
            float speed;
            bool stale;
            std::string name;
            float controlVal;
        };

        // Singleton instance
        static StepperViewer* instance;

        StepperViewer() = default;

        // Maps stepper IDs to their structure
        std::map<uint8_t, Stepper> steppers;

    public:
        // Get singleton instance
        static StepperViewer* getInstance();

        // Overridden render function
        void render() override;

        // Can be used to set the configuration of steppers. Takes in a map of stepper IDs, and their associated names
        void setHardwareConfig(const std::map<uint8_t, std::string>& ids);

        // Callback for RCP
        void receiveRCPUpdate(const RCP_TwoFloat& state);

        // Custom reset
        void reset() override;

        ~StepperViewer() override = default;
    };
}

#endif //STEPPERCONTROL_H
