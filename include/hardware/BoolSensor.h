#ifndef BOOLSENSOR_H
#define BOOLSENSOR_H

#include <map>
#include <set>

#include "HardwareQualifier.h"
#include "utils.h"

// Hardware singleton for the BoolSensors
namespace LRI::RCI {
    class BoolSensors {
        // How frequently to request refreshes from the target
    public:
        struct BoolSensorState;

    private:
        // Storage for sensor states, mapped to their qualifiers
        std::map<HardwareQualifier, BoolSensorState*> state;

        // Timer for refreshing
        StopWatch refreshTimer;
        float refreshTime = 5.0f;

        BoolSensors() = default;
        ~BoolSensors();

    public:
        struct BoolSensorState {
            bool open = false;
            bool stale = true;
        };

        // Get the singleton
        static BoolSensors* getInstance();

        // Receive updates from RCP
        void receiveRCPUpdate(const HardwareQualifier& qual, bool newstate);

        // Set which qualifiers are tracked by the singleton, and the interval for refreshes
        void setHardwareConfig(const std::set<HardwareQualifier>& ids, int _refreshTime = 5);

        // Clears storage and resets class to defaults
        void reset();

        // Gets a pointer that can be tracked and stored by viewer classes
        [[nodiscard]] const BoolSensorState* getState(const HardwareQualifier& qual) const;

        // Request a refresh of all tracked qualifiers
        void refreshAll() const;

        // Called from main to check timer and refresh state
        void update();
    };
} // namespace LRI::RCI

#endif // BOOLSENSOR_H
