#ifndef SENSORVIEWER_H
#define SENSORVIEWER_H

#include <map>
#include <vector>

#include "WModule.h"
#include "hardware/HardwareQualifier.h"
#include "hardware/Sensors.h"

namespace LRI::RCI {
    // A window module which shows graphs logging received sensor datapoints, or an abridged
    // sensor output which is just the sensor values as text
    class SensorViewer : public WModule {
        /*
         * These two structures are used for encoding the naming and rendering information for
         * a particular device class. This data is stored in the GRAPHINFO map. For each
         * device class:
         *  - There is a vector holding a series of graphs. Multiple seperate graphs are needed
         *    in the case that a sensor returns different data types that cannot be graphed on the
         *    same set of axes (ie, gps data, power monitor data)
         *  - Each graph stores its name, its Y axis name (all X axes are time), and a vector of lines
         *  - Each line is an individual line on a graph. Up to 3 lines can be graphed per graph. This
         *    is used in the case that a sensor returns multiple values of the same unit that make sense
         *    to graph on the same set of axes, for example, an accelerometer
         *  - Each line stores its name, as well as which "data channel" of the sensor it represents. These
         *    correspond to one of the 4 doubles in the Sensors::DataPoint::data array, as each of the doubles
         *    represents a different channel of data from a particular sensor
         *
         * All this data is stored in the GRAPHINFO map, whose definition is present in the SensorViewer.cpp
         * file. It is truly a horrendous sight to behold, but its the best way I could think of to encode all
         * this data in the program. It does allow for some very clean loops that actually render the graphs,
         * though, as opposed to the previous system (in v1.0.x) which has a bunch of special cases for each
         * device class.
         */
        struct Line {
            const char* name;
            const int datanum;
        };

        struct Graph {
            const char* name;
            const char* axis;
            const std::vector<Line> lines;
        };

        static const std::map<RCP_DeviceClass, std::vector<Graph>> GRAPHINFO;

        // Takes a qualifier and a datapoint and renders it to a string with the correct units
        static std::string renderLatestReadingsString(const HardwareQualifier& qual, const Sensors::DataPoint& data);


        static int CLASSID;
        const int classid;

        // An ampty data point
        static constexpr Sensors::DataPoint empty{0, 0, 0, 0, 0};

        // If the sensors should be displayed in abridged mode
        const bool abridged;

        // Holds the data vector pointers mapped to their qualifiers. This pointer is updated by the Sensors class
        std::map<HardwareQualifier, const std::vector<Sensors::DataPoint>*> sensors;

        // Render all the graphs for a particular qualifier
        static void renderGraphs(const HardwareQualifier& qual, const std::vector<Sensors::DataPoint>* data,
                                 const ImVec2& plotsize);

        // The tare and clear states for each sensor
        std::map<HardwareQualifier, StopWatch[4]> tarestate;
        std::map<HardwareQualifier, StopWatch> clearState;

    public:
        explicit SensorViewer(const std::set<HardwareQualifier>& quals, bool abridged = false);
        ~SensorViewer() override = default;

        // Overridden render function
        void render() override;
    };
}

#endif //SENSORVIEWER_H
