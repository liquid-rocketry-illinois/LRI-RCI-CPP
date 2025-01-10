#ifndef SENSORREADINGS_H
#define SENSORREADINGS_H

#include <map>
#include <string>
#include <thread>
#include <vector>

#include "BaseUI.h"
#include "RCP_Host/RCP_Host.h"

namespace LRI::RCI {
    struct SensorQualifier {
        RCP_DeviceClass_t devclass = 0;
        uint8_t id = 0;
        std::string name;
        bool operator<(SensorQualifier const& rhf) const;
        [[nodiscard]] std::string asString() const;
    };

    struct FileWriteThreadData {
        std::thread* thread;
        std::atomic_bool done;
    };

    struct DataPoint {
        double timestamp;
        union {
            double singleVal;
            double axisData[3];
            double gpsData[4];
        } data;
    };

    class SensorReadings : public BaseUI {
        static constexpr int DATA_VECTOR_INITIAL_SIZE = 500'000;
        static const std::map<RCP_DeviceClass_t, std::string> DEVCLASS_NAMES;

        static void toCSVFile(SensorQualifier qual, std::vector<DataPoint>* data, std::atomic_bool* done);
        static SensorReadings* instance;

        SensorReadings() = default;

        std::map<SensorQualifier, std::vector<DataPoint>> sensors;
        std::map<SensorQualifier, FileWriteThreadData> filewritethreads;

    public:
        static SensorReadings* getInstance();

        void render() override;
        void setHardwareConfig(const std::set<SensorQualifier>& sensids);
        void receiveRCPUpdate(const SensorQualifier& qual, const DataPoint& data);

        ~SensorReadings() override = default;
    };
}

#endif //SENSORREADINGS_H
