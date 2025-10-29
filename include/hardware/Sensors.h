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
namespace LRI::RCI::Sensors {
    // Initial size for the vectors storing the sensor data
    static constexpr int DATA_VECTOR_INITIAL_SIZE = 5'000;

    // A single point of sensor data. The data field is used differently depending on the devclass
    struct DataPoint {
        double timestamp;
        double data[4];
    };

    // Receivers for different data structures from RCP
    int receiveRCPUpdate(const RCP_OneFloat& data);
    int receiveRCPUpdate(const RCP_TwoFloat& data);
    int receiveRCPUpdate(const RCP_ThreeFloat& data);
    int receiveRCPUpdate(const RCP_FourFloat& data);

    // Sets which sensors are active
    void setHardwareConfig(const std::set<HardwareQualifier>& sensids);

    // Clears all data vectors and the vector maps
    void reset();

    // To be called by main. Handles destruction of threads
    void update();

    // Get a pointer that viewer classes can use to track a sensor
    [[nodiscard]] const std::vector<DataPoint>* getState(const HardwareQualifier& qual);

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
} // namespace LRI::RCI::Sensors

#endif // SENSORS_H
