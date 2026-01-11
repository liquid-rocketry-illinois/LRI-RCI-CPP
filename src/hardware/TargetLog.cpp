#include "hardware/TargetLog.h"

#include "hardware/EventLog.h"

namespace LRI::RCI::TargetLog {
    static std::string latest;

    RCP_Error receiveRCPUpdate(RCP_TargetLogData data) {
        EventLog::getGlobalLog().addTargetLog(data);
        latest += std::string(data.data, data.length);
        return RCP_ERR_SUCCESS;
    }

    void clearDisplayString() {
        latest = "";
    }

    const std::string& getDisplayString() { return latest; }
} // namespace LRI::RCI::RawData
