#include "hardware/RawData.h"

namespace LRI::RCI::RawData {
    static std::stringstream chars;

    void receiveRCPUpdate(const RCP_CustomData& data) {
        std::stringstream temp;
        for(int i = 0; i < data.length; i++) {
            temp << static_cast<const char*>(data.data)[i];
        }

        chars << temp.str();
    }

    void reset() { chars.str(""); }

    const std::stringstream& getData() { return chars; }
} // namespace LRI::RCI::RawData
