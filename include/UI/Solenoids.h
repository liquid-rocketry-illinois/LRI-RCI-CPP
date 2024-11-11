#ifndef SOLENOIDS_H
#define SOLENOIDS_H

#include <map>
#include <set>
#include <RCP_Host/RCP_Host.h>

#include "BaseUI.h"

namespace LRI::RCI {
    class Solenoids : public BaseUI {
        static Solenoids* instance;

        std::map<uint8_t, bool> solUpdated;
        std::map<uint8_t, bool> sols;
        Solenoids() = default;

    public:
        static Solenoids* const getInstance();
        void render() override;
        void setHardwareConfig(const std::set<uint8_t>* solIds);
        void receiveRCPUpdate(const uint8_t id, RCP_SolenoidState_t state);

        ~Solenoids() override = default;
    };
}

#endif //SOLENOIDS_H
