#include <ranges>

#include "hardware/BoolSensor.h"

namespace LRI::RCI {
    BoolSensors::~BoolSensors() {
        reset();
    }

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
        for(const auto& qual : state | std::views::keys) {
            RCP_requestDeviceReadID(qual.devclass, qual.id);
            state.at(qual)->stale = true;
        }
    }

    const BoolSensors::BoolSensorState* BoolSensors::getState(const HardwareQualifier& qual) const {
        return state.at(qual);
    }

    void BoolSensors::setHardwareConfig(const std::set<HardwareQualifier>& ids) {
        reset();

        for(const auto& qual : ids) {
            state[qual] = new BoolSensorState();
        }

        refreshAll();
    }
}