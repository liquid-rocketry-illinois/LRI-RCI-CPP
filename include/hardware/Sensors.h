#ifndef SENSORS_H
#define SENSORS_H

#include <atomic>
#include <map>
#include <mutex>
#include <set>
#include <thread>
#include <vector>

#include "HardwareQualifier.h"

// Singleton for representing sensors. This is a much more complex class due
// to all the different devices it handles
namespace LRI::RCI {
    class Sensors {
    public:
        // A single point of sensor data. The data field is used differently depending on the devclass
        struct DataPoint {
            double timestamp;
            double data[4];
        };

    private:
        // Initial size for the vectors storing the sensor data
        static constexpr int DATA_VECTOR_INITIAL_SIZE = 5'000;

        // Helper function that is ran in a thread to write sensor data to a CSV file. The vector pointer is
        // de-allocated!
        void toCSVFile(const HardwareQualifier& qual, const std::vector<DataPoint>* data);

        // Holds the data vectors mapped to their qualifiers
        std::map<HardwareQualifier, std::vector<DataPoint>> sensors;

        // Threads are placed in a map that maps the thread ID to its pointer, so threads can move themselves
        // between these two structures
        std::map<std::thread::id, std::thread*> activeThreads;
        std::map<std::thread::id, std::thread*> destroyThreads;

        // Lock for the above maps
        std::mutex threadSetMux;

        // A flag for all threads to indicate if they should self destruct
        std::atomic_bool destroy;

        Sensors() = default;
        ~Sensors();

    public:
        // Get singleton instance
        static Sensors* getInstance();

        // Receivers for different data structures from RCP
        void receiveRCPUpdate(const RCP_OneFloat& data);
        void receiveRCPUpdate(const RCP_TwoFloat& data);
        void receiveRCPUpdate(const RCP_ThreeFloat& data);
        void receiveRCPUpdate(const RCP_FourFloat& data);

        // Sets which sensors are active
        void setHardwareConfig(const std::set<HardwareQualifier>& sensids);

        // Clears all data vectors and the vector maps
        void reset();

        // To be called by main. Handles destruction of threads
        void update();

        // Get a pointer that viewer classes can use to track a sensor
        [[nodiscard]] const std::vector<DataPoint>* getState(const HardwareQualifier& qual) const;

        // Request to write a sensors data to CSV
        void writeCSV(const HardwareQualifier& qual);

        // Request to tare a sensor
        void tare(const HardwareQualifier& qual, uint8_t dataChannel);

        // Request to clear the data vector of a particular sensor
        void clearGraph(const HardwareQualifier& qual);

        // Clear all graphs
        void clearAll();

        // Remove a qualifier from the sensor tracking list and delete all its data
        void removeSensor(const HardwareQualifier& qual);

        // Adds a single qualifier
        void addSensor(const HardwareQualifier& qual);
    };
} // namespace LRI::RCI

#endif // SENSORS_H
