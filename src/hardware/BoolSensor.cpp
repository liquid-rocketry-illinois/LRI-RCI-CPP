#include <ranges>

#include "hardware/BoolSensor.h"
#include "hardware/TestState.h"

namespace LRI::RCI {
    BoolSensors::~BoolSensors() { reset(); }

    BoolSensors* BoolSensors::getInstance() {
        static BoolSensors instance;
        return &instance;
    }

    void BoolSensors::receiveRCPUpdate(const HardwareQualifier& qual, bool newstate) {
        if(!state.contains(qual)) return;
        state[qual]->open = newstate;
        state[qual]->stale = false;
    }

    void BoolSensors::reset() {
        for(const BoolSensorState* s : state | std::views::values) delete s;
        state.clear();
    }

    void BoolSensors::refreshAll() const {
        for(const auto& qual : state | std::views::keys) {
            RCP_requestGeneralRead(RCP_DEVCLASS_BOOL_SENSOR, qual.id);
            state.at(qual)->stale = true;
        }
    }

    const BoolSensors::BoolSensorState* BoolSensors::getState(const HardwareQualifier& qual) const {
        if(!state.contains(qual)) return nullptr;
        return state.at(qual);
    }

    void BoolSensors::setHardwareConfig(const std::set<HardwareQualifier>& ids, int _refreshTime) {
        reset();

        refreshTime = std::max(_refreshTime, 1);

        for(const auto& qual : ids) {
            state[qual] = new BoolSensorState();
        }

        refreshAll();
        refreshTimer.reset();
    }

    void BoolSensors::update() {
        if(!TestState::getInited()) return;
        if(refreshTimer.timeSince() > refreshTime) {
            refreshAll();
            refreshTimer.reset();
        }
    }

} // namespace LRI::RCI
