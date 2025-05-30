#include "hardware/RawData.h"

namespace LRI::RCI {
    RawData* RawData::getInstance() {
        static RawData* instance = nullptr;
        if(instance == nullptr) instance = new RawData();
        return instance;
    }

    void RawData::receiveRCPUpdate(const RCP_CustomData& data) {
        std::stringstream temp;
        for(int i = 0; i < data.length; i++) {
            temp << static_cast<char*>(data.data)[i];
        }

        chars << temp.str();
    }

    void RawData::reset() { chars.str(""); }

    const std::stringstream& RawData::getData() const { return chars; }
} // namespace LRI::RCI
