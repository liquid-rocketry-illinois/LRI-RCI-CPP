#ifndef SENSORREADINGS_H
#define SENSORREADINGS_H

#include <map>
#include <string>
#include <thread>
#include <vector>

#include "BaseUI.h"
#include "RCP_Host/RCP_Host.h"

namespace LRI::RCI {
    // Structure that is used to represent a sensor. Contains the device class, ID if applicable, and a display string
    struct SensorQualifier {
        RCP_DeviceClass_t devclass = 0;
        uint8_t id = 0;
        std::string name;

        // Used for ordering
        bool operator<(SensorQualifier const& rhf) const;

        // Helper for packing data as a string
        [[nodiscard]] std::string asString() const;
    };

    // A single point of data. Contains a timestamp and the raw data, which can be interpreted as a single value, a
    // list of 3 values (for example, accelerometer data with 3 axes), and a list of 4 values (for GPS data)
    struct DataPoint {
        double timestamp;
        double data[4];
    };

    // A window which shows graphs logging received sensor datapoints
    class SensorReadings : public BaseUI {
        // Structure for combining a thread and an atomic done flag
        struct FileWriteThreadData {
            std::thread* thread;
            std::atomic_bool done;
        };

        static constexpr ImGuiWindowFlags fullscreenFlags = ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoMove |
                                                  ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize |
                                                  ImGuiWindowFlags_NoDecoration;

        static constexpr ImGuiWindowFlags regularFlags = ImGuiWindowFlags_None;


        // Initial size for the vectors storing the sensor data
        static constexpr int DATA_VECTOR_INITIAL_SIZE = 50'000;

        // Helper function that is ran in a thread to write sensor data to a CSV file. The vector pointer is de-allocated!
        static void toCSVFile(const SensorQualifier& qual, const std::vector<DataPoint>* data, std::atomic_bool* done);

        static void renderLatestReadings(const SensorQualifier& qual, const DataPoint& data);

        // Singleton instance
        static SensorReadings* instance;

        SensorReadings() = default;

        // Holds the data vectors mapped to their qualifiers
        std::map<SensorQualifier, std::vector<DataPoint>> sensors;

        // Holds the file writing threads mapped to sensor qualifiers
        std::map<SensorQualifier, FileWriteThreadData> filewritethreads;

        void drawSensors();

        bool fullscreen;
        bool doResize = false;
        ImVec2 preFullscreenSize;
        ImVec2 preFullscreeenPos;

    public:
        // Get singleton instance
        static SensorReadings* getInstance();

        // Overridden render function
        void render() override;

        // Can be used to set the configuration of sensors. Takes in a set of qualifiers that represent which sensors
        // should be allocated
        void setHardwareConfig(const std::set<SensorQualifier>& sensids);

        // Callback for RCP
        void receiveRCPUpdate(const RCP_OneFloat& data);
        void receiveRCPUpdate(const RCP_TwoFloat& data);
        void receiveRCPUpdate(const RCP_ThreeFloat& data);
        void receiveRCPUpdate(const RCP_FourFloat& data);

        // Needs custom reset
        void reset() override;

        ~SensorReadings() override = default;
    };
}

#endif //SENSORREADINGS_H
