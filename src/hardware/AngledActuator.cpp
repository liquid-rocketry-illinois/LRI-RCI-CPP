#include "hardware/AngledActuator.h"

#include "RCP_Host/RCP_Host.h"

#include "hardware/HardwareControl.h"

namespace LRI::RCI::AngledActuators {
    static std::set<HardwareQualifier> actuators;

    void reset() {
        for(const auto& qual : actuators) Sensors::removeSensor(qual);
        actuators.clear();
    }

    void setHardwareConfig(std::set<HardwareQualifier>& quals) {
        reset();
        actuators.insert(quals.cbegin(), quals.cend());
    }

    void setActuatorPos(const HardwareQualifier& qual, float degrees) {
        if(!actuators.contains(qual)) {
            HWCTRL::addError({HWCTRL::ErrorType::HWNE_HOST, qual});
            return;
        }
        RCP_sendAngledActuatorWrite(qual.id, degrees);
    }

    const std::vector<Sensors::DataPoint>* getState(const HardwareQualifier& qual) {
        if(!actuators.contains(qual)) {
            HWCTRL::addError({HWCTRL::ErrorType::HWNE_HOST, qual});
            return nullptr;
        }
        return Sensors::getState(qual);
    }

    void refreshAll() {
        for(const auto& qual : actuators) {
            RCP_requestGeneralRead(RCP_DEVCLASS_ANGLED_ACTUATOR, qual.id);
        }
    }
} // namespace LRI::RCI::AngledActuators
