#ifndef SOLENOIDS_H
#define SOLENOIDS_H

#include <map>
#include <set>
#include <string>
#include <utils.h>
#include <RCP_Host/RCP_Host.h>
#include "BaseUI.h"

namespace LRI::RCI {
    class Solenoids : public BaseUI {
        static Solenoids* instance;
        static constexpr ImVec2 STATUS_SQUARE_SIZE = ImVec2(20, 20);
        static constexpr int buttonSize = 50;
        static constexpr int buttonLeftMargin = 30;
        static constexpr int buttonTopMargin = 35;
        static constexpr int solsPerRow = 4;
        static constexpr int buttonExtraMargin = 100;

        std::map<uint8_t, bool> solUpdated;
        std::map<uint8_t, bool> sols;
        std::map<uint8_t, std::string> solname;

        Solenoids() = default;

    public:
        static Solenoids* const getInstance();
        void render() override;
        void setHardwareConfig(const std::map<uint8_t, std::string>& solIds);
        void receiveRCPUpdate(const RCP_SolenoidData& data);

        ~Solenoids() override = default;
    };
}

#endif //SOLENOIDS_H
