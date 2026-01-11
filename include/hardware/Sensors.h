#ifndef SENSORS_H
#define SENSORS_H

#include <set>

#include "EventLog.h"
#include "HardwareQualifier.h"

// Singleton for representing sensors. This is a much more complex class due
// to all the different devices it handles
namespace LRI::RCI::Sensors {
    // Receivers for different data structures from RCP
    // These first two take const references since 1Fs are shared with AAs, and 2Fs are shared with steppers
    RCP_Error receiveRCPUpdate1(const RCP_1F& data);
    RCP_Error receiveRCPUpdate2(const RCP_2F& data);
    RCP_Error receiveRCPUpdate3(RCP_3F data);
    RCP_Error receiveRCPUpdate4(RCP_4F data);

    // Sets which sensors are active
    void setHardwareConfig(const std::set<HardwareQualifier>& sensids);

    // Get a pointer that viewer classes can use to track a sensor
    [[nodiscard]] const float* getLatestState(const HardwareChannel& ch);
    [[nodiscard]] FloatData getFullLog(const HardwareChannel& ch);

    // Request to tare a sensor
    void tare(const HardwareChannel& qual);

    // Clears all data vectors and the vector maps
    void reset();
} // namespace LRI::RCI::Sensors

#endif // SENSORS_H
