#ifndef SOLENOIDSVIEWER_H
#define SOLENOIDSVIEWER_H

#include <map>
#include <set>
#include "Windowlet.h"

#include "hardware/EventLog.h"
#include "hardware/HardwareQualifier.h"

namespace LRI::RCI {
    // A window module for showing and controlling simple actuator status
    class DiscreteActuatorViewer : public WModule {
        // If a refresh button should be shown at the top
        const bool refreshButton;

        // Be evil?
        bool evilMode;
        bool prevChordState;

        // Maps pointers to actuator states updated by SimpleActuators to their qualifiers
        std::map<HardwareQualifier, EventLog::TargetUint> acts;

    public:
        explicit DiscreteActuatorViewer(const std::set<HardwareQualifier>& quals, bool refreshButton = true);

        // Overridden render function
        void render() override;

        ~DiscreteActuatorViewer() override = default;
    };
} // namespace LRI::RCI

#endif // SOLENOIDSVIEWER_H
