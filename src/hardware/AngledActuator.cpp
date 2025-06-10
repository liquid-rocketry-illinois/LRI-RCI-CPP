#include <ranges>

#include "RCP_Host/RCP_Host.h"

#include "hardware/AngledActuator.h"

namespace LRI::RCI {
    AngledActuators* AngledActuators::getInstance() {
        static AngledActuators* instance = nullptr;
        if(instance == nullptr) instance = new AngledActuators();
        return instance;
    }

    void AngledActuators::setHardwareConfig(std::set<HardwareQualifier>& quals) {
        for(const auto& qual : actuators | std::views::keys) Sensors::getInstance()->removeSensor(qual);
        actuators.clear();

        for(const auto& qual : quals) {
            Sensors::getInstance()->addSensor(qual);
            actuators[qual] = Sensors::getInstance()->getState(qual);
        }
    }

    void AngledActuators::setActuatorPos(const HardwareQualifier& qual, float degrees) {
        RCP_requestAngledActuatorWrite(qual.id, degrees);
    }

    const std::vector<Sensors::DataPoint>* AngledActuators::getState(const HardwareQualifier& qual) {
        return Sensors::getInstance()->getState(qual);
    }

} // namespace LRI::RCI
