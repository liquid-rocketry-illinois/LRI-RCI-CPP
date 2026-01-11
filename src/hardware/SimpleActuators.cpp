#include "hardware/SimpleActuators.h"

#include <ranges>

#include "hardware/HardwareControl.h"

namespace LRI::RCI::SimpleActuators {
    // Maps qualifiers to state pointers
    static std::map<HardwareQualifier, SimpleActuatorState> state;

    void setHardwareConfig(const std::set<HardwareQualifier>& solIds) {
        reset();

        for(const auto& qual : solIds)
            state[qual] = {false, true};
        refreshAll();
    }

    const SimpleActuatorState* getLatestState(const HardwareQualifier& qual) {
        if(!state.contains(qual)) {
            HWCTRL::addError({HWCTRL::ErrorType::HWNE_HOST, qual});
            return nullptr;
        }

        return &state[qual];
    }

    BoolData getFullLog(const HardwareChannel& ch) {
        if(!state.contains(ch)) {
            HWCTRL::addError({HWCTRL::ErrorType::HWNE_HOST, ch});
            return {};
        }

        return {&EventLog::getGlobalLog().getSensorTimestamps().at(ch), &EventLog::getGlobalLog().getBools().at(ch)};
    }

    void setActuatorState(const HardwareQualifier& qual, RCP_SimpleActuatorState newState) {
        if(!state.contains(qual)) {
            HWCTRL::addError({HWCTRL::ErrorType::HWNE_HOST, qual});
            return;
        }

        state[qual].stale = true;
        EventLog::getGlobalLog().addSActWrite(qual.id, newState);
        RCP_sendSimpleActuatorWrite(qual.id, newState);
    }

    void reset() { state.clear(); }

    void refreshAll() {
        for(const auto& qual : state | std::views::keys) {
            RCP_requestGeneralRead(RCP_DEVCLASS_SIMPLE_ACTUATOR, qual.id);
            EventLog::getGlobalLog().addReadReq(qual);
            state[qual].stale = true;
        }
    }

    RCP_Error receiveRCPUpdate(RCP_SimpleActuatorData data) {
        HardwareQualifier qual{RCP_DEVCLASS_SIMPLE_ACTUATOR, data.ID};
        if(!state.contains(qual)) {
            HWCTRL::addError({HWCTRL::ErrorType::HWNE_TARGET, qual});
            return RCP_ERR_INVALID_DEVCLASS;
        }

        state[qual].state = data.state == RCP_SIMPLE_ACTUATOR_ON;
        state[qual].stale = false;
        EventLog::getGlobalLog().addSimpleActuator(data);
        return RCP_ERR_SUCCESS;
    }
} // namespace LRI::RCI::SimpleActuators
