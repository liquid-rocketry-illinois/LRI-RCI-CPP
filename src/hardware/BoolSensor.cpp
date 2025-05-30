#include <ranges>

#include "hardware/BoolSensor.h"
#include "hardware/TestState.h"

namespace LRI::RCI {
    BoolSensors::~BoolSensors() { reset(); }

    BoolSensors* BoolSensors::getInstance() {
        static BoolSensors* instance = nullptr;
        if(instance == nullptr) instance = new BoolSensors();
        return instance;
    }

    void BoolSensors::receiveRCPUpdate(const HardwareQualifier& qual, bool newstate) {
        state[qual]->open = newstate;
        state[qual]->stale = false;
    }

    void BoolSensors::reset() {
        for(const BoolSensorState* s : state | std::views::values) delete s;
        state.clear();
    }

    void BoolSensors::refreshAll() const {
        if(!TestState::getInited()) return;
        for(const auto& qual : state | std::views::keys) {
            RCP_requestSensorDeviceRead(qual.devclass, qual.id);
            state.at(qual)->stale = true;
        }
    }

    const BoolSensors::BoolSensorState* BoolSensors::getState(const HardwareQualifier& qual) const {
        return state.at(qual);
    }

    void BoolSensors::setHardwareConfig(const std::set<HardwareQualifier>& ids, int _refreshTime) {
        reset();

        refreshTime = max(_refreshTime, 1);

        for(const auto& qual : ids) {
            state[qual] = new BoolSensorState();
        }

        refreshAll();
        refreshTimer.reset();
    }

    void BoolSensors::update() {
        if(refreshTimer.timeSince() > refreshTime) {
            refreshAll();
            refreshTimer.reset();
        }
    }

} // namespace LRI::RCI
