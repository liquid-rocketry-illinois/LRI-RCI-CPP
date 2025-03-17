#ifndef SOLENOIDS_H
#define SOLENOIDS_H

#include <map>
#include <set>
#include <string>
#include <utils.h>
#include <RCP_Host/RCP_Host.h>
#include "BaseUI.h"

namespace LRI::RCI {

    // A window for showing and controlling solenoid status
    class SolenoidViewer : public BaseUI {
        // Singleton instance
        static SolenoidViewer* instance;

        // Maps whether data is stale to a solenoid ID
        std::map<uint8_t, bool> solUpdated;

        // Maps the current state of a solenoid to its ID
        std::map<uint8_t, bool> sols;

        // Maps human identifiable names to solenoid ID
        std::map<uint8_t, std::string> solname;

        SolenoidViewer() = default;

    public:
        // Get singleton instance
        static SolenoidViewer* getInstance();

        // Overridden render function
        void render() override;

        // Can be used to set the IDs and names of the present solenoids
        void setHardwareConfig(const std::map<uint8_t, std::string>& solIds);

        // Callback for RCP
        void receiveRCPUpdate(const RCP_SolenoidData& data);

        ~SolenoidViewer() override = default;
    };
}

#endif //SOLENOIDS_H
