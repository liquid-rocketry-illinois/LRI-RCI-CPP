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

    int BoolSensors::receiveRCPUpdate(const HardwareQualifier& qual, bool newstate) {
        if(!state.contains(qual)) return (qual.devclass << 8) | qual.id;
        state[qual].open = newstate;
        state[qual].stale = false;
        return 0;
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
        if(!state.contains(qual)) throw HWNE("Bool Sensor does not exist", qual);
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
