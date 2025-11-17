#include "hardware/RawData.h"

namespace LRI::RCI::RawData {
    static std::stringstream chars;

    int receiveRCPUpdate(RCP_CustomData data) {
        for(int i = 0; i < data.length; i++)
            chars << static_cast<const char*>(data.data)[i];
        return 0;
    }

    void reset() { chars.str(""); }

    const std::stringstream& getData() { return chars; }
} // namespace LRI::RCI::RawData
