#include "hardware/RawData.h"

namespace LRI::RCI {
    RawData* RawData::instance;

    RawData* RawData::getInstance() {
        if(instance == nullptr) instance = new RawData();
        return instance;
    }

    void RawData::receiveRCPUpdate(const RCP_CustomData& data) {
        raw.insert(raw.end(), static_cast<uint8_t*>(data.data), static_cast<uint8_t*>(data.data) + data.length);
    }

    void RawData::reset() {
        raw.clear();
    }

    const std::vector<uint8_t>* RawData::getData() const {
        return &raw;
    }
}
