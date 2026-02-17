#include "hardware/Motors.h"

#include <map>
#include <ranges>

#include "hardware/HardwareControl.h"

namespace LRI::RCI::Motors {
    std::map<HardwareQualifier, Motor> motors;

    const Motor* getState(const HardwareQualifier& qual) {
        if(!motors.contains(qual)) {
            HWCTRL::addError({HWCTRL::ErrorType::HWNE_HOST, qual});
            return nullptr;
        }

        return &motors.at(qual);
    }

    void setState(const HardwareQualifier& qual, float value) {
        if(!motors.contains(qual)) {
            HWCTRL::addError({HWCTRL::ErrorType::HWNE_HOST, qual});
            return;
        }

        RCP_sendMotorWrite(qual.id, value);
        motors[qual].stale = true;
    }

    int receiveRCPUpdate(const HardwareQualifier& qual, const float& value) {
        if(!motors.contains(qual)) {
            HWCTRL::addError({HWCTRL::ErrorType::HWNE_TARGET, qual});
            return 1;
        }

        motors[qual].value = value;
        motors[qual].stale = true;
        return 0;
    }

    void setHarwareConfig(const std::set<HardwareQualifier>& quals) {
        reset();

        for(const auto& qual : quals) motors[qual] = Motor();

        refreshAll();
    }

    void reset() {
        motors.clear();
    }

    void refreshAll() {
        for(const auto& qual : motors | std::views::keys) {
            RCP_requestGeneralRead(RCP_DEVCLASS_STEPPER, qual.id);
            motors.at(qual).stale = true;
        }
    }
} // namespace LRI::RCI::Motors
