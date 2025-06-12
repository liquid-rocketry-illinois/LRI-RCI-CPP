#ifndef ANGLEDACTUATORVIEWER_H
#define ANGLEDACTUATORVIEWER_H

#include <map>
#include <set>
#include <vector>

#include "UI/WModule.h"

#include "hardware/AngledActuator.h"
#include "hardware/HardwareQualifier.h"
#include "hardware/Sensors.h"

namespace LRI::RCI {

    // Viewer class for Angled Actuators
    class AngledActuatorViewer : public WModule {

        // Pointers to latest actuator positions
        std::map<HardwareQualifier, const std::vector<Sensors::DataPoint>*> actuators;

        // Floats for inputs
        std::map<HardwareQualifier, float> setpoints;

    public:
        explicit AngledActuatorViewer(const std::set<HardwareQualifier>& quals);

        void render() override;

        ~AngledActuatorViewer() override = default;
    };
} // namespace LRI::RCI

#endif // ANGLEDACTUATORVIEWER_H
