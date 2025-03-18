#ifndef SENSORS_H
#define SENSORS_H

#include "HardwareQualifier.h"
#include <vector>
#include <map>
#include <set>
#include <atomic>
#include <thread>
#include <mutex>

namespace LRI::RCI {
    class Sensors {
        struct DataPoint;

        // Initial size for the vectors storing the sensor data
        static constexpr int DATA_VECTOR_INITIAL_SIZE = 5'000;

        // Helper function that is ran in a thread to write sensor data to a CSV file. The vector pointer is de-allocated!
        void toCSVFile(const HardwareQualifier& qual, const std::vector<DataPoint>* data);

        // Holds the data vectors mapped to their qualifiers
        std::map<HardwareQualifier, std::vector<DataPoint>*> sensors;

        // Holds the file writing threads mapped to sensor qualifiers
        std::map<std::thread::id, std::thread*> activeThreads;
        std::map<std::thread::id, std::thread*> destroyThreads;
        std::mutex threadSetMux;
        std::atomic_bool destroy;

        Sensors() = default;
        ~Sensors();

    public:
        struct DataPoint {
            double timestamp;
            double data[4];
        };

        static Sensors* getInstance();

        void receiveRCPUpdate(const RCP_OneFloat& data);
        void receiveRCPUpdate(const RCP_TwoFloat& data);
        void receiveRCPUpdate(const RCP_ThreeFloat& data);
        void receiveRCPUpdate(const RCP_FourFloat& data);
        void setHardwareConfig(const std::set<HardwareQualifier>& sensids);

        void reset();
        void update();

        [[nodiscard]] const std::map<HardwareQualifier, std::vector<DataPoint>*>* getState() const;
        [[nodiscard]] const std::vector<DataPoint>* getState(const HardwareQualifier& qual) const;
        void writeCSV(const HardwareQualifier& qual);
    };
}

#endif //SENSORS_H
