#include <ranges>

#include "RCP_Host/RCP_Host.h"
#include "hardware/Steppers.h"

namespace LRI::RCI {
    Steppers* Steppers::getInstance() {
        static Steppers instance;
        return &instance;
    }

    Steppers::~Steppers() {
        for(const Stepper* s : motors | std::views::values) delete s;
        motors.clear();
    }

    const Steppers::Stepper* Steppers::getState(const HardwareQualifier& qual) const {
        if(!motors.contains(qual)) return nullptr;
        return motors.at(qual);
    }

    void Steppers::receiveRCPUpdate(const HardwareQualifier& qual, const float& pos, const float& speed) {
        if(!motors.contains(qual)) return;
        motors[qual]->position = pos;
        motors[qual]->speed = speed;
        motors[qual]->stale = true;
    }

    void Steppers::refreshAll() const {
        for(const auto& qual : motors | std::views::keys) {
            RCP_requestGeneralRead(RCP_DEVCLASS_STEPPER, qual.id);
            motors.at(qual)->stale = true;
        }
    }

    void Steppers::setHardwareConfig(const std::set<HardwareQualifier>& motorlist) {
        reset();

        for(const auto& qual : motorlist) {
            motors[qual] = new Stepper();
        }

        refreshAll();
    }

    void Steppers::reset() {
        for(const Stepper* s : motors | std::views::values) delete s;
        motors.clear();
    }

    void Steppers::setState(const HardwareQualifier& qual, RCP_StepperControlMode controlMode, float value) {
        if(!motors.contains(qual)) return;
        RCP_sendStepperWrite(qual.id, controlMode, value);
        motors[qual]->stale = true;
    }
} // namespace LRI::RCI
