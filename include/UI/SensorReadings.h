#ifndef SENSORREADINGS_H
#define SENSORREADINGS_H

#include <map>
#include <string>
#include <vector>

#include "BaseUI.h"
#include "RCP_Host/RCP_Host.h"

namespace LRI::RCI {
    struct SensorQualifier {
        RCP_DeviceClass_t devclass;
        uint8_t id = 0;
        std::string name;

        bool operator<(SensorQualifier const& lhf, SensorQualifier const& rhf) const {
            if(lhf.devclass == rhf.devclass) return lhf.id < rhf.id;
            return lhf.devclass < rhf.devclass;
        }
    };

    struct DataPoint {
        uint32_t timestamp;
        union {
            int32_t int32val;
            int32_t axisData[3];
            int64_t gpsData[4];
        } data;
    };

    class SensorReadings : public BaseUI {
        static constexpr ImVec2 STATUS_SQUARE_SIZE = ImVec2(20, 20);
        static constexpr int DATA_VECTOR_INITIAL_SIZE = 500'000;

        static SensorReadings* instance;

        SensorReadings() = default;

        std::map<SensorQualifier, std::vector<DataPoint>> sensors;

    public:
        static SensorReadings* const getInstance();

        void render() override;
        void setHardwareConfig(const std::set<SensorQualifier>& sensids);
        void receiveRCPUpdate(const SensorQualifier& qual, const DataPoint& data);

        ~SensorReadings() override = default;
    };
}

#endif //SENSORREADINGS_H
