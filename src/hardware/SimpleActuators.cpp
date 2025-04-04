#include <ranges>

#include "hardware/SimpleActuators.h"

namespace LRI::RCI {
    SimpleActuators* SimpleActuators::getInstance() {
        static SimpleActuators* instance = nullptr;
        if(instance == nullptr) instance = new SimpleActuators();
        return instance;
    }

    SimpleActuators::~SimpleActuators() {
        reset();
    }

    void SimpleActuators::receiveRCPUpdate(const HardwareQualifier& qual, bool newState) {
        state[qual]->stale = false;
        state[qual]->open = newState;
    }

    void SimpleActuators::setHardwareConfig(const std::set<HardwareQualifier>& solIds) {
        reset();

        for(const auto& qual : solIds) {
            state[qual] = new ActuatorState();
        }
    }

    void SimpleActuators::reset() {
        for(const ActuatorState* s : state | std::views::values) delete s;
        state.clear();
    }

    const SimpleActuators::ActuatorState* SimpleActuators::getState(const HardwareQualifier& qual) const {
        return state.at(qual);
    }

    void SimpleActuators::refreshAll() const {
        for(const auto& qual : state | std::views::keys) {
            RCP_requestSimpleActuatorRead(qual.id);
            state.at(qual)->stale = true;
        }
    }

    void SimpleActuators::setActuatorState(const HardwareQualifier& qual, RCP_SimpleActuatorState newState) {
        state[qual]->stale = true;
        RCP_sendSimpleActuatorWrite(qual.id, newState);
    }
}
