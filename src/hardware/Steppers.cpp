#include <ranges>

#include "hardware/Steppers.h"
#include "hardware/Solenoids.h"

namespace LRI::RCI {
    Steppers* Steppers::instance;

    Steppers* Steppers::getInstance() {
        if(instance == nullptr) instance = new Steppers();
        return instance;
    }

    const std::map<HardwareQualifier, Steppers::Stepper*>* Steppers::getState() const {
        return &motors;
    }

    const Steppers::Stepper* Steppers::getState(const HardwareQualifier& qual) const {
        return motors.at(qual);
    }

    void Steppers::receiveRCPUpdate(const HardwareQualifier& qual, const float& pos, const float& speed) {
        motors[qual]->position = pos;
        motors[qual]->speed = speed;
        motors[qual]->stale = true;
    }

    void Steppers::refreshAll() const {
        for(const auto& qual : motors | std::views::keys) {
            RCP_requestStepperRead(qual.id);
        }
    }

    void Steppers::setHardwareConfig(const std::vector<HardwareQualifier>& motorlist) {
        for(const Stepper* s : motors | std::views::values) delete s;
        motors.clear();

        for(const auto& qual : motorlist) {
            motors[qual] = new Stepper();
        }
    }

    void Steppers::setState(const HardwareQualifier& qual, RCP_StepperControlMode_t controlMode, float value) {
        motors[qual]->controlMode = controlMode;
        motors[qual]->controlVal = value;
        RCP_sendStepperWrite(qual.id, controlMode, &value);
    }
}