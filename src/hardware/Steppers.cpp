#include "hardware/Steppers.h"

#include <map>
#include <ranges>

#include "RCP_Host/RCP_Host.h"

#include "hardware/HardwareControl.h"

namespace LRI::RCI::Steppers {
    // Storage container for steppers
    std::map<HardwareQualifier, Stepper> motors;

    void setHardwareConfig(const std::set<HardwareQualifier>& motorlist) {
        reset();

        for(const auto& qual : motorlist) motors[qual] = Stepper();
        refreshAll();
    }

    const Stepper* getLatestState(const HardwareQualifier& qual) {
        if(!motors.contains(qual)) {
            HWCTRL::addError({HWCTRL::ErrorType::HWNE_HOST, qual});
            return nullptr;
        }

        return &motors.at(qual);
    }

    FloatData getFullLog(const HardwareChannel& ch) {
        if(!motors.contains(ch)) {
            HWCTRL::addError({HWCTRL::ErrorType::HWNE_HOST, ch});
            return {};
        }

        return {&EventLog::getGlobalLog().getSensorTimestamps().at(ch), &EventLog::getGlobalLog().getFloats().at(ch)};
    }

    void setState(const HardwareQualifier& qual, RCP_StepperControlMode controlMode, float value) {
        if(!motors.contains(qual)) {
            HWCTRL::addError({HWCTRL::ErrorType::HWNE_HOST, qual});
            return;
        }

        RCP_sendStepperWrite(qual.id, controlMode, value);
        EventLog::getGlobalLog().addStepperWrite(qual.id, controlMode, value);
        motors[qual].stale = true;
    }

    void reset() { motors.clear(); }

    void refreshAll() {
        for(const auto& qual : motors | std::views::keys) {
            RCP_requestGeneralRead(RCP_DEVCLASS_STEPPER, qual.id);
            motors.at(qual).stale = true;
            EventLog::getGlobalLog().addReadReq(qual);
        }
    }

    RCP_Error receiveRCPUpdate(const RCP_2F& data) {
        HardwareQualifier qual = {RCP_DEVCLASS_STEPPER, data.ID};
        if(!motors.contains(qual)) {
            HWCTRL::addError({HWCTRL::ErrorType::HWNE_TARGET, qual});
            return RCP_ERR_INVALID_DEVCLASS;
        }

        motors[qual].position = data.data[0];
        motors[qual].speed = data.data[1];
        motors[qual].stale = true;
        EventLog::getGlobalLog().add2F(data);
        return RCP_ERR_SUCCESS;
    }
} // namespace LRI::RCI::Steppers
