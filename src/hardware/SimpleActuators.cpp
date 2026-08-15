#include "hardware/SimpleActuators.h"

#include <ranges>

#include "hardware/HardwareControl.h"

namespace LRI::RCI::SimpleActuators {
    // Maps qualifiers to state pointers
    static std::map<HardwareQualifier, ActuatorState> state;

    int receiveRCPUpdate(RCP_ByteData data) {
        HardwareQualifier qual{RCP_DEVCLASS_DISCRETE_ACTUATOR, data.ID};
        if(!state.contains(qual)) {
            HWCTRL::addError({HWCTRL::ErrorType::HWNE_TARGET, qual});
            return 1;
        }

        state[qual].stale = false;
        state[qual].open = data.data > 0;
        return 0;
    }

    void setHardwareConfig(const std::set<HardwareQualifier>& solIds) {
        reset();

        for(const auto& qual : solIds) {
            state[qual] = ActuatorState();
        }

        refreshAll();
    }

    void reset() { state.clear(); }

    const ActuatorState* getState(const HardwareQualifier& qual) {
        if(!state.contains(qual)) {
            HWCTRL::addError({HWCTRL::ErrorType::HWNE_HOST, qual});
            return nullptr;
        }

        return &state.at(qual);
    }

    void refreshAll() {
        for(const auto& qual : state | std::views::keys) {
            RCP_requestGeneralRead(RCP_DEVCLASS_DISCRETE_ACTUATOR, qual.id);
            state.at(qual).stale = true;
        }
    }

    void setActuatorState(const HardwareQualifier& qual, uint8_t newState) {
        if(!state.contains(qual)) {
            HWCTRL::addError({HWCTRL::ErrorType::HWNE_HOST, qual});
            return;
        }

        state[qual].stale = true;
        RCP_sendDiscreteActuatorWrite(qual.id, newState);
    }
} // namespace LRI::RCI::SimpleActuators
