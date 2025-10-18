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
        if(!state.contains(qual)) throw HWNE("Sensor qualifier does not exist: " + qual.asString());
        state[qual].open = newstate;
        state[qual].stale = false;
    }

    void BoolSensors::reset() {
        state.clear();
    }

    void BoolSensors::refreshAll() {
        for(const auto& qual : state | std::views::keys) {
            RCP_requestGeneralRead(RCP_DEVCLASS_BOOL_SENSOR, qual.id);
            state.at(qual).stale = true;
        }
    }

    const BoolSensors::BoolSensorState* BoolSensors::getState(const HardwareQualifier& qual) const {
        if(!state.contains(qual)) throw HWNE("Bool Sensor does not exist: " + qual.asString());
        return &state.at(qual);
    }

    void BoolSensors::setHardwareConfig(const std::set<HardwareQualifier>& ids, int _refreshTime) {
        reset();

        refreshTime = std::max(_refreshTime, 1);

        for(const auto& qual : ids) {
            state[qual] = BoolSensorState();
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
