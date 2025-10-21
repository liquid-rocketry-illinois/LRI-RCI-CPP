#include <ranges>

#include "hardware/SimpleActuators.h"

namespace LRI::RCI {
    SimpleActuators* SimpleActuators::getInstance() {
        static SimpleActuators instance;
        return &instance;
    }

    SimpleActuators::~SimpleActuators() { reset(); }

    void SimpleActuators::receiveRCPUpdate(const HardwareQualifier& qual, bool newState) {
        if(!state.contains(qual)) return;
        state[qual].stale = false;
        state[qual].open = newState;
    }

    void SimpleActuators::setHardwareConfig(const std::set<HardwareQualifier>& solIds) {
        reset();

        for(const auto& qual : solIds) {
            state[qual] = ActuatorState();
        }

        refreshAll();
    }

    void SimpleActuators::reset() {
        state.clear();
    }

    const SimpleActuators::ActuatorState* SimpleActuators::getState(const HardwareQualifier& qual) const {
        if(!state.contains(qual)) return nullptr;
        return &state.at(qual);
    }

    void SimpleActuators::refreshAll() {
        for(const auto& qual : state | std::views::keys) {
            RCP_requestGeneralRead(RCP_DEVCLASS_SIMPLE_ACTUATOR, qual.id);
            state.at(qual).stale = true;
        }
    }

    void SimpleActuators::setActuatorState(const HardwareQualifier& qual, RCP_SimpleActuatorState newState) {
        if(!state.contains(qual)) return;
        state[qual].stale = true;
        RCP_sendSimpleActuatorWrite(qual.id, newState);
    }
} // namespace LRI::RCI
