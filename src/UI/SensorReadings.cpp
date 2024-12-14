#include <utility>

#include "UI/SensorReadings.h"

namespace LRI::RCI {
    bool SensorQualifier::operator<(SensorQualifier const& rhf) const {
        if(devclass == rhf.devclass) return id < rhf.id;
        return devclass < rhf.devclass;
    }

    SensorReadings* SensorReadings::instance;

    SensorReadings* const SensorReadings::getInstance() {
        if(instance == nullptr) instance = new SensorReadings();
        return instance;
    }

    void SensorReadings::render() {
    }

    void SensorReadings::setHardwareConfig(const std::set<SensorQualifier>& sensids) {
        sensors.clear();
        for(const auto& qual : sensids) {
            sensors[qual] = std::vector<DataPoint>(DATA_VECTOR_INITIAL_SIZE);
        }
    }

    void SensorReadings::receiveRCPUpdate(const SensorQualifier& qual, const DataPoint& data) {
        sensors[qual].push_back(data);
    }
}
