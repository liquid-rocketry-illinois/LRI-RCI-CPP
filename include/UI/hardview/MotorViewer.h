#ifndef LRI_CONTROL_PANEL_MOTORVIEWER_H
#define LRI_CONTROL_PANEL_MOTORVIEWER_H

#include <set>
#include <map>

#include "WModule.h"
#include "hardware/HardwareQualifier.h"
#include "hardware/Motors.h"

namespace LRI::RCI {
    class MotorViewer : public WModule {
        const bool refreshButton;

        std::map<HardwareQualifier, const Motors::Motor*> states;
        std::map<HardwareQualifier, float> inputs;

    public:
        explicit MotorViewer(const std::set<HardwareQualifier>& quals, bool refreshButton);
        ~MotorViewer() override = default;

        void render() override;
    };
}

#endif // LRI_CONTROL_PANEL_MOTORVIEWER_H
