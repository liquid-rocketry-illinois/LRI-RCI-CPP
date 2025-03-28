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

        // This map contains how each sensor should be rendered. It is quite a mess. clang-format is turned off
        // here since it looks neater this way than the way the formatter would make it

        // The map maps each sensor device class to a vector of graphs. Each graph contains the name of the graph,
        // what its Y axis should be (all X axes are time), and a vector of lines to be plotted. Each line contains
        // the name of the line, as well as the offset into the Sensors::DataPoint::Data array. It looks messy, but
        // this system is significantly cleaner than the previous implementation. As in half the number of LOC cleaner.

        // clang-format off
        static constexpr std::map<RCP_DeviceClass_t, std::vector<Graph>> GRAPHINFO {
            {RCP_DEVCLASS_AM_PRESSURE, {{.name = "Ambient Pressure", "Pressure (mbars)", {{.name = "Pressure", .datanum = 0}}}}},
            {RCP_DEVCLASS_AM_TEMPERATURE, {{.name = "Ambient Temperature", "Temperature (Celsius)", {{.name = "Temperature", .datanum = 0}}}}},
            {RCP_DEVCLASS_PRESSURE_TRANSDUCER, {{.name = "Pressure", "Pressure (psi)", {{.name = "Pressure", .datanum = 0}}}}},
            {RCP_DEVCLASS_RELATIVE_HYGROMETER, {{.name = "Relative Humidity", "Humidity (Relative %)", {{.name = "Humidity", .datanum = 0}}}}},
            {RCP_DEVCLASS_LOAD_CELL, {{.name = "Load Cell", "Mass (kg)", {{.name = "Mass", .datanum = 0}}}}},
            {RCP_DEVCLASS_POWERMON, {{.name = "Power Monitor - Voltage", "Voltage (V)", {{.name = "Volts", .datanum = 0}}}, {.name = "Power Monitor - Power", "Power (W)", {{.name = "Power", .datanum = 1}}}}},
            {RCP_DEVCLASS_ACCELEROMETER, {{.name = "Accelerometer", "Acceleration (m/s/s)", {{.name = "X", .datanum = 0}, {.name = "Y", .datanum = 1}, {.name = "Z", .datanum = 2}}}}},
            {RCP_DEVCLASS_GYROSCOPE, {{.name = "Gyroscope", "Angular Acceleration (deg/s/s)", {{.name = "X", .datanum = 0}, {.name = "Y", .datanum = 1}, {.name = "Z", .datanum = 2}}}}},
            {RCP_DEVCLASS_MAGNETOMETER, {{.name = "Magnetometer", "Magnetic Field (Gauss)", {{.name = "X", .datanum = 0}, {.name = "Y", .datanum = 1}, {.name = "Z", .datanum = 2}}}}},
            {RCP_DEVCLASS_GPS, {{.name = "GPS - Lat & Lon", "Position (degrees)", {{.name = "Latitude", .datanum = 0}, {.name = "Longitude", .datanum = 1}}}, {.name = "GPS - Altitude", "Altitude (m)", {{.name = "Altitude", .datanum = 2}}}, {.name = "GPS - Ground Speed", "Speed (m/s)", {{.name = "Speed", .datanum = 3}}}}},
        };
        // clang-format on

        const int classid;
        // Holds the data vectors mapped to their qualifiers
        std::map<HardwareQualifier, const std::vector<Sensors::DataPoint>*> sensors;
        std::map<HardwareQualifier, bool> abridged;

        void renderGraphs(const HardwareQualifier& qual, const std::vector<Sensors::DataPoint>* data,
                          const ImVec2& plotsize) const;

    public:
        explicit SensorViewer(const std::map<HardwareQualifier, bool>& quals);
        ~SensorViewer() override = default;

        // Overridden render function
        void render() override;
    };
}

#endif //SENSORVIEWER_H
