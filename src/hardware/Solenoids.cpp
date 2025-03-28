#include <ranges>

#include "hardware/Solenoids.h"

namespace LRI::RCI {
    Solenoids* Solenoids::getInstance() {
        static Solenoids* instance = nullptr;
        if(instance == nullptr) instance = new Solenoids();
        return instance;
    }

    Solenoids::~Solenoids() {
        reset();
    }


    void Solenoids::receiveRCPUpdate(const HardwareQualifier& qual, bool newState) {
        state[qual]->stale = false;
        state[qual]->open = newState;
    }

    void Solenoids::setHardwareConfig(const std::set<HardwareQualifier>& solIds) {
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

    const Solenoids::SolenoidState* Solenoids::getState(const HardwareQualifier& qual) const {
        return state.at(qual);
    }

    void Solenoids::refreshAll() const {
        for(const auto& qual : state | std::views::keys) {
            RCP_requestSolenoidRead(qual.id);
            state.at(qual)->stale = true;
        }
    }

    void Solenoids::setSolenoidState(const HardwareQualifier& qual, RCP_SolenoidState_t newState) {
        state[qual]->stale = true;
        RCP_sendSolenoidWrite(qual.id, newState);
    }
}
