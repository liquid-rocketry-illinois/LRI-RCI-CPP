#ifndef TESTCONTROL_H
#define TESTCONTROL_H

#include <utils.h>
#include <RCP_Host/RCP_Host.h>

#include "BaseUI.h"

namespace LRI::RCI {

    // A window for controlling the test state of the target
    class TestControl : public BaseUI {
        // Singleton instance
        static TestControl* instance;

        // The currently running test state
        RCP_TestRunningState_t testState;

        // The current test number
        int testNumber;

        // Whether sensor data should be streamed
        bool dataStreaming;

        // Whether heartbeats are enabled
        bool doHeartbeats;

        // The heartbeat rate (seconds between heartbeats)
        int heartbeatRate;

        // The modified hearbeat rate
        int inputHeartbeatRate;

        // Stopwatch for heartbeats
        StopWatch heartbeat;

        TestControl();

    public:
        // Get singleton instance
        static TestControl* getInstance();

        // Overridden render function
        void render() override;

        // Callback for RCP
        void receiveRCPUpdate(const RCP_TestData& data);

        // Custom reset
        void reset() override;

        ~TestControl() override = default;
    };
}

#endif //TESTCONTROL_H
