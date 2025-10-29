#include "hardware/BoolSensor.h"

#include <ranges>

#include "hardware/HardwareControl.h"
#include "hardware/TestState.h"

namespace LRI::RCI::BoolSensors {
    // Storage for sensor states, mapped to their qualifiers
    static std::map<HardwareQualifier, BoolSensorState> state;

    // Timer for refreshing
    static StopWatch refreshTimer;
    static float refreshTime = 5.0f;

    int receiveRCPUpdate(const HardwareQualifier& qual, bool newstate) {
        if(!state.contains(qual)) {
            HWCTRL::addError({HWCTRL::ErrorType::HWNE_TARGET, qual});
            return 1;
        }

        state[qual].open = newstate;
        state[qual].stale = false;
        return 0;
    }

    void reset() { state.clear(); }

    void refreshAll() {
        for(const auto& qual : state | std::views::keys) {
            RCP_requestGeneralRead(RCP_DEVCLASS_BOOL_SENSOR, qual.id);
            state.at(qual).stale = true;
        }
    }

    const BoolSensorState* getState(const HardwareQualifier& qual) {
        if(!state.contains(qual)) {
            HWCTRL::addError({HWCTRL::ErrorType::HWNE_HOST, qual});
            return nullptr;
        }
        return &state.at(qual);
    }

    void setHardwareConfig(const std::set<HardwareQualifier>& ids, int _refreshTime) {
        reset();

        refreshTime = static_cast<float>(std::max(_refreshTime, 1));

        for(const auto& qual : ids) {
            state[qual] = BoolSensorState();
        }

        refreshAll();
        refreshTimer.reset();
    }

    void update() {
        if(!TestState::getInited()) return;
        if(refreshTimer.timeSince() > refreshTime) {
            refreshAll();
            refreshTimer.reset();
        }
    }

} // namespace LRI::RCI::BoolSensors
