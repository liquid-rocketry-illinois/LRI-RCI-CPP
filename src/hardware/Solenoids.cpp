#include <ranges>

#include "hardware/Solenoids.h"

namespace LRI::RCI {
    Solenoids* Solenoids::instance;

    Solenoids* Solenoids::getInstance() {
        if(instance == nullptr) instance = new Solenoids();
        return instance;
    }

    void Solenoids::receiveRCPUpdate(const HardwareQualifier& qual, bool newState) {
        state[qual]->stale = false;
        state[qual]->open = newState;
    }

    void Solenoids::setHardwareConfig(const std::vector<HardwareQualifier>& solIds) {
        for(const SolenoidState* s : state | std::views::values) delete s;
        state.clear();

        for(const auto& qual : solIds) {
            state[qual] = new SolenoidState();
        }
    }

    void Solenoids::reset() {
        for(const SolenoidState* s : state | std::views::values) delete s;
        state.clear();
    }

    const std::map<HardwareQualifier, Solenoids::SolenoidState*>* Solenoids::getState() const {
        return &state;
    }

    const Solenoids::SolenoidState* Solenoids::getState(const HardwareQualifier& qual) const {
        return state.at(qual);
    }

    void Solenoids::refreshAll() const {
        for(const auto& qual : state | std::views::keys) {
            RCP_requestSolenoidRead(qual.id);
        }
    }

    void Solenoids::setSolenoidState(const HardwareQualifier& qual, RCP_SolenoidState_t newState) {
        state[qual]->stale = true;
        RCP_sendSolenoidWrite(qual.id, newState);
    }
}
