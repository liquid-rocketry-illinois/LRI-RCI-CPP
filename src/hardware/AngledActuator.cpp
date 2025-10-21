#include "hardware/AngledActuator.h"

#include "RCP_Host/RCP_Host.h"

#include "hardware/HardwareControl.h"

namespace LRI::RCI {
    AngledActuators* AngledActuators::getInstance() {
        static AngledActuators instance;
        return &instance;
    }

    void AngledActuators::reset() {
        for(const auto& qual : actuators) Sensors::getInstance()->removeSensor(qual);
        actuators.clear();
    }

    void AngledActuators::setHardwareConfig(std::set<HardwareQualifier>& quals) {
        reset();
        actuators.insert(quals.cbegin(), quals.cend());
    }

    void AngledActuators::setActuatorPos(const HardwareQualifier& qual, float degrees) {
        if(!actuators.contains(qual)) {
            HWCTRL::addError({HWCTRL::ErrorType::HWNE_HOST, qual});
            return;
        }
        RCP_sendAngledActuatorWrite(qual.id, degrees);
    }

    const std::vector<Sensors::DataPoint>* AngledActuators::getState(const HardwareQualifier& qual) const {
        if(!actuators.contains(qual)) {
            HWCTRL::addError({HWCTRL::ErrorType::HWNE_HOST, qual});
            return nullptr;
        }
        return Sensors::getInstance()->getState(qual);
    }

    void AngledActuators::refreshAll() {
        for(const auto& qual : actuators) {
            RCP_requestGeneralRead(RCP_DEVCLASS_ANGLED_ACTUATOR, qual.id);
        }
    }
} // namespace LRI::RCI
