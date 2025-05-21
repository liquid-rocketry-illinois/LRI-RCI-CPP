#ifndef SOLENOIDSVIEWER_H
#define SOLENOIDSVIEWER_H

#include <map>
#include <set>
#include "WModule.h"
#include "hardware/HardwareQualifier.h"
#include "hardware/SimpleActuators.h"

namespace LRI::RCI {
    // A window module for showing and controlling simple actuator status
    class SimpleActuatorViewer : public WModule {
        // If a refresh button should be shown at the top
        const bool refreshButton;

        // Maps pointers to actuator states updated by SimpleActuators to their qualifiers
        std::map<HardwareQualifier, const SimpleActuators::ActuatorState*> sols;

    public:
        explicit SimpleActuatorViewer(const std::set<HardwareQualifier>&& quals, bool refreshButton = true);

        // Overridden render function
        void render() override;

        ~SimpleActuatorViewer() override = default;
    };
}

#endif //SOLENOIDSVIEWER_H
