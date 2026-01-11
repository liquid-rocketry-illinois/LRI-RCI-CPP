#include "hardware/AngledActuator.h"

#include <ranges>

#include "RCP_Host/RCP_Host.h"

#include "hardware/EventLog.h"
#include "hardware/HardwareControl.h"

namespace LRI::RCI::AngledActuators {
    static std::map<HardwareQualifier, AngledActuatorState> actuators;

    void setHardwareConfig(const std::set<HardwareQualifier>& quals) {
        reset();
        for(const auto& qual : quals) actuators[qual] = {0, true};
    }

    const AngledActuatorState* getLatestState(const HardwareChannel& ch) {
        if(!actuators.contains(ch)) {
            HWCTRL::addError({HWCTRL::ErrorType::HWNE_HOST, ch});
            return nullptr;
        }
        return &actuators[ch];
    }

    FloatData getFullLog(const HardwareChannel& ch) {
        if(!actuators.contains(ch)) {
            HWCTRL::addError({HWCTRL::ErrorType::HWNE_HOST, ch});
            return {};
        }
        return {&EventLog::getGlobalLog().getSensorTimestamps().at(ch), &EventLog::getGlobalLog().getFloats().at(ch)};
    }

    void setActuatorPos(const HardwareQualifier& qual, float degrees) {
        if(!actuators.contains(qual)) {
            HWCTRL::addError({HWCTRL::ErrorType::HWNE_HOST, qual});
            return;
        }

        RCP_sendAngledActuatorWrite(qual.id, degrees);
        actuators[qual].stale = true;
        EventLog::getGlobalLog().addAActWrite(qual.id, degrees);
    }

    void reset() {
        actuators.clear();
    }

    void refreshAll() {
        for(const auto& qual : actuators | std::views::keys) {
            RCP_requestGeneralRead(RCP_DEVCLASS_ANGLED_ACTUATOR, qual.id);
            actuators[qual].stale = true;
            EventLog::getGlobalLog().addReadReq(qual);
        }
    }

    RCP_Error receiveRCPUpdate(const RCP_1F& f1) {
        HardwareQualifier qual = {f1.devclass, f1.ID};
        if(!actuators.contains(qual)) {
            HWCTRL::addError({HWCTRL::ErrorType::HWNE_TARGET, qual});
            return RCP_ERR_INVALID_DEVCLASS;
        }

        actuators[qual].value = f1.data;
        actuators[qual].stale = false;
        EventLog::getGlobalLog().add1F(f1);
        return RCP_ERR_SUCCESS;
    }
} // namespace LRI::RCI::AngledActuators
