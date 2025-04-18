#ifndef BOOLSENSOR_H
#define BOOLSENSOR_H

#include <map>
#include <set>

#include "utils.h"
#include "HardwareQualifier.h"

// Hardware singleton for the BoolSensors
namespace LRI::RCI {
    class BoolSensors {
        // How frequently to request refreshes from the target
        static constexpr int REFRESH_TIME = 5;

        struct BoolSensorState;

        // Storage for sensor states, mapped to their qualifiers
        std::map<HardwareQualifier, BoolSensorState*> state;

        // Timer for refreshing
        StopWatch refreshTimer;

        BoolSensors() = default;
        ~BoolSensors();

    public:
        struct BoolSensorState {
            bool open;
            bool stale;
        };

        // Get the singleton
        static BoolSensors* getInstance();

        // Receive updates from RCP
        void receiveRCPUpdate(const HardwareQualifier& qual, bool newstate);

        // Set which qualifiers are tracked by the singleton
        void setHardwareConfig(const std::set<HardwareQualifier>& ids);

        // Clears storage and resets class to defaults
        void reset();

        // Gets a pointer that can be tracked and stored by viewer classes
        [[nodiscard]] const BoolSensorState* getState(const HardwareQualifier& qual) const;

        // Request a refresh of all tracked qualifiers
        void refreshAll() const;

        // Called from main to check timer and refresh state
        void update();
    };
}

#endif //BOOLSENSOR_H
