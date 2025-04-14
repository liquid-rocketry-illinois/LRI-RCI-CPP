#ifndef BOOLSENSORVIEWER_H
#define BOOLSENSORVIEWER_H

#include <set>

#include "WModule.h"

#include "hardware/HardwareQualifier.h"
#include "hardware/BoolSensor.h"

namespace LRI::RCI {
    class BoolSensorViewer : public WModule {
        static int CLASSID;

        const int classid;
        const bool refreshButton;

        std::map<HardwareQualifier, const BoolSensors::BoolSensorState*> sensors;

    public:
        explicit BoolSensorViewer(const std::set<HardwareQualifier>& quals, bool refreshButton = false);

        void render() override;

        ~BoolSensorViewer() override = default;
    };
}

#endif //BOOLSENSORVIEWER_H
