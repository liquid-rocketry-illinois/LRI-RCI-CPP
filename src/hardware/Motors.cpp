#include "hardware/Motors.h"

#include "hardware/HardwareControl.h"

namespace LRI::RCI::Motors {
    std::set<HardwareQualifier> motors;

    const std::vector<Sensors::DataPoint>* getState(const HardwareQualifier& qual) {
        if(!motors.contains(qual)) {
            HWCTRL::addError({HWCTRL::ErrorType::HWNE_HOST, qual});
            return nullptr;
        }

        return Sensors::getState(qual);
    }

    void setState(const HardwareQualifier& qual, float value) {
        if(!motors.contains(qual)) {
            HWCTRL::addError({HWCTRL::ErrorType::HWNE_HOST, qual});
            return;
        }

        RCP_sendMotorWrite(qual.id, value);
    }

    void setHarwareConfig(const std::set<HardwareQualifier>& quals) {
        reset();
        motors.insert(quals.begin(), quals.end());
        refreshAll();
    }

    void reset() {
        for(const auto& qual : motors) Sensors::removeSensor(qual);
        motors.clear();
    }

    void refreshAll() {
        for(const auto& qual : motors) {
            RCP_requestGeneralRead(RCP_DEVCLASS_MOTOR, qual.id);
        }
    }
} // namespace LRI::RCI::Motors
