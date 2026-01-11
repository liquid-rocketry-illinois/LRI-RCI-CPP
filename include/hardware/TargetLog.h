#ifndef RAWDATA_H
#define RAWDATA_H

#include <sstream>

#include "RCP_Host/RCP_Host.h"

// Singleton class for the CUSTOM device class. Technically not fully in spec
// with RCP, but for our purposes of just using it as a debug log, this does
// everything necessary
namespace LRI::RCI::TargetLog {
    // Receive new chars from RCP
    RCP_Error receiveRCPUpdate(RCP_TargetLogData data);

    void clearDisplayString();

    // Return stream for display
    [[nodiscard]] const std::string& getDisplayString();
} // namespace LRI::RCI

#endif // RAWDATA_H
