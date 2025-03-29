#ifndef SENSORVIEWER_H
#define SENSORVIEWER_H

#include <map>
#include <vector>

#include "WModule.h"
#include "hardware/HardwareQualifier.h"
#include "hardware/Sensors.h"

namespace LRI::RCI {
    // A window which shows graphs logging received sensor datapoints
    class SensorViewer : public WModule {
        struct Line {
            const char* name;
            const int datanum;
        };

        struct Graph {
            const char* name;
            const char* axis;
            const std::vector<Line> lines;
        };

        static void renderLatestReadings(const HardwareQualifier& qual, const Sensors::DataPoint& data);
        static int CLASSID;
        static constexpr Sensors::DataPoint empty{0, 0, 0, 0, 0};

        static const std::map<RCP_DeviceClass_t, std::vector<Graph>> GRAPHINFO;
        static const std::map<RCP_DeviceClass_t, std::vector<Graph>> test;

        const int classid;
        const bool abridged;
        // Holds the data vectors mapped to their qualifiers
        std::map<HardwareQualifier, const std::vector<Sensors::DataPoint>*> sensors;

        static void renderGraphs(const HardwareQualifier& qual, const std::vector<Sensors::DataPoint>* data,
                          const ImVec2& plotsize);

    public:
        explicit SensorViewer(const std::set<HardwareQualifier>& quals, bool abridged = false);
        ~SensorViewer() override = default;

        // Overridden render function
        void render() override;
    };
}

#endif //SENSORVIEWER_H
