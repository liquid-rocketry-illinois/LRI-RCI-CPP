#include "hardware/RawData.h"

namespace LRI::RCI::RawData {
    static std::stringstream chars;
    static std::string latest;

    int receiveRCPUpdate(RCP_CustomData data) {
        for(int i = 0; i < data.length; i++)
            chars << static_cast<const char*>(data.data)[i];
        latest = chars.str();
        return 0;
    }

    void reset() { chars.str(""); }

    const std::string& getData() { return latest; }
} // namespace LRI::RCI::RawData
