#include "hardware/Steppers.h"

#include <ranges>
#include "RCP_Host/RCP_Host.h"

#include "hardware/HardwareControl.h"

namespace LRI::RCI {
    Steppers* Steppers::getInstance() {
        static Steppers instance;
        return &instance;
    }

    Steppers::~Steppers() {
        motors.clear();
    }

    const Steppers::Stepper* Steppers::getState(const HardwareQualifier& qual) const {
        if(!motors.contains(qual)) {
            HWCTRL::addError({HWCTRL::ErrorType::HWNE_HOST, qual});
            return nullptr;
        }

        return &motors.at(qual);
    }

    int Steppers::receiveRCPUpdate(const HardwareQualifier& qual, const float& pos, const float& speed) {
        if(!motors.contains(qual)) {
            HWCTRL::addError({HWCTRL::ErrorType::HWNE_TARGET, qual});
            return 1;
        }

        motors[qual].position = pos;
        motors[qual].speed = speed;
        motors[qual].stale = true;
        return 0;
    }

    void Steppers::refreshAll() {
        for(const auto& qual : motors | std::views::keys) {
            RCP_requestGeneralRead(RCP_DEVCLASS_STEPPER, qual.id);
            motors.at(qual).stale = true;
        }
    }

    void Steppers::setHardwareConfig(const std::set<HardwareQualifier>& motorlist) {
        reset();

        for(const auto& qual : motorlist) {
            motors[qual] = Stepper();
        }

        refreshAll();
    }

    void Steppers::reset() {
        motors.clear();
    }

    void Steppers::setState(const HardwareQualifier& qual, RCP_StepperControlMode controlMode, float value) {
        if(!motors.contains(qual)) {
            HWCTRL::addError({HWCTRL::ErrorType::HWNE_HOST, qual});
            return;
        }

        RCP_sendStepperWrite(qual.id, controlMode, value);
        motors[qual].stale = true;
    }
} // namespace LRI::RCI
