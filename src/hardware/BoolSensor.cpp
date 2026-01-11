#include "hardware/BoolSensor.h"

#include <ranges>

#include "hardware/EventLog.h"
#include "hardware/HardwareControl.h"
#include "hardware/TestState.h"

namespace LRI::RCI::BoolSensors {
    // Storage for sensor states, mapped to their qualifiers
    static std::map<HardwareQualifier, BoolSensorState> state;

    // Timer for refreshing
    static StopWatch refreshTimer;
    static float refreshTime = 5.0f;

    void setHardwareConfig(const std::set<HardwareQualifier>& ids, int _refreshTime) {
        reset();

        refreshTime = static_cast<float>(std::max(_refreshTime, 1));

        for(const auto& qual : ids) state[qual] = {false, true};
        refreshAll();
        refreshTimer.reset();
    }

    const BoolSensorState* getLatestState(const HardwareChannel& ch) {
        if(!state.contains(ch)) {
            HWCTRL::addError({HWCTRL::ErrorType::HWNE_HOST, ch});
            return nullptr;
        }
        return &state.at(ch);
    }

    BoolData getFullLog(const HardwareChannel& ch) {
        if(!state.contains(ch)) {
            HWCTRL::addError({HWCTRL::ErrorType::HWNE_HOST, ch});
            return {};
        }

        return {&EventLog::getGlobalLog().getSensorTimestamps().at(ch), &EventLog::getGlobalLog().getBools().at(ch)};
    }

    void reset() { state.clear(); }

    void refreshAll() {
        for(const auto& qual : state | std::views::keys) {
            RCP_requestGeneralRead(RCP_DEVCLASS_BOOL_SENSOR, qual.id);
            state[qual].stale = true;
            EventLog::getGlobalLog().addReadReq(qual);
        }
    }

    RCP_Error receiveRCPUpdate(RCP_BoolData data) {
        HardwareQualifier qual{RCP_DEVCLASS_BOOL_SENSOR, data.ID};
        if(!state.contains(qual)) {
            HWCTRL::addError({HWCTRL::ErrorType::HWNE_TARGET, qual});
            return RCP_ERR_INVALID_DEVCLASS;
        }

        state[qual].state = data.data;
        state[qual].stale = false;
        EventLog::getGlobalLog().addBoolData(data);
        return RCP_ERR_SUCCESS;
    }

    void update() {
        if(!TestState::getInited()) return;
        if(refreshTimer.timeSince() > refreshTime) {
            refreshAll();
            refreshTimer.reset();
        }
    }

} // namespace LRI::RCI::BoolSensors
