#include "UI/Solenoids.h"

#include "RCP_Host/RCP_Host.h"

namespace LRI::RCI {
    Solenoids* Solenoids::instance;

    Solenoids* const Solenoids::getInstance() {
        if(instance == nullptr) instance = new Solenoids();
        return instance;
    }

    void Solenoids::render() {

    }

    void Solenoids::setHardwareConfig(const std::set<uint8_t>* solIds) {
        sols.clear();
        solUpdated.clear();

        for(const auto& i : *solIds) {
            sols[i] = false;
            solUpdated[i] = false;
            RCP_requestSolenoidRead(i);
        }
    }

    void Solenoids::receiveRCPUpdate(const uint8_t id, RCP_SolenoidState_t state) {
        sols[id] = state == SOLENOID_ON;
        solUpdated[id] = true;
    }



}
