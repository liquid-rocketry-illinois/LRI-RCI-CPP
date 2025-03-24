#ifndef SOLENOIDS_H
#define SOLENOIDS_H

#include <map>
#include <set>
#include "BaseUI.h"
#include "hardware/HardwareQualifier.h"
#include "hardware/Solenoids.h"

namespace LRI::RCI {
    // A window for showing and controlling solenoid status
    class SolenoidViewer : public BaseUI {
        const bool refreshButton;
        // Maps human identifiable names to solenoid ID
        std::map<HardwareQualifier, const Solenoids::SolenoidState*> sols;

    public:
        SolenoidViewer(const std::set<HardwareQualifier>& sols, bool refreshButton = true);

        // Overridden render function
        void render() override;

        ~SolenoidViewer() override = default;
    };
}

#endif //SOLENOIDS_H
