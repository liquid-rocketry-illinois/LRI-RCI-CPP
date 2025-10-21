#include <ranges>

#include "RCP_Host/RCP_Host.h"

#include "hardware/AngledActuator.h"

namespace LRI::RCI {
    AngledActuators* AngledActuators::getInstance() {
        static AngledActuators instance;
        return &instance;
    }

    void AngledActuators::setHardwareConfig(std::set<HardwareQualifier>& quals) {
        for(const auto& qual : actuators) Sensors::getInstance()->removeSensor(qual);
        actuators.clear();
        actuators.insert(quals.cbegin(), quals.cend());
    }

    void AngledActuators::setActuatorPos(const HardwareQualifier& qual, float degrees) {
        RCP_sendAngledActuatorWrite(qual.id, degrees);
    }

    const std::vector<Sensors::DataPoint>* AngledActuators::getState(const HardwareQualifier& qual) const {
        return Sensors::getInstance()->getState(qual);
    }

    void AngledActuators::refreshAll() {
        for(const auto& qual : actuators) {
            RCP_requestGeneralRead(RCP_DEVCLASS_ANGLED_ACTUATOR, qual.id);
        }
    }
} // namespace LRI::RCI
