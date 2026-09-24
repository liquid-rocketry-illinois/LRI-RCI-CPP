#ifndef LRI_CONTROL_PANEL_MOTORVIEWER_H
#define LRI_CONTROL_PANEL_MOTORVIEWER_H

#include <set>
#include <map>

#include "Windowlet.h"
#include "hardware/HardwareQualifier.h"
#include "hardware/EventLog.h"

namespace LRI::RCI {
    class MotorViewer : public WModule {
        struct SData {
            EventLog::TargetFloat data;
            float input;
        };

        const bool refreshButton;

        std::map<HardwareQualifier, SData> states;

    public:
        explicit MotorViewer(const std::set<HardwareQualifier>& quals, bool refreshButton);
        ~MotorViewer() override = default;

        void render() override;
    };
}

#endif // LRI_CONTROL_PANEL_MOTORVIEWER_H
