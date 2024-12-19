#ifndef TESTCONTROL_H
#define TESTCONTROL_H

#include <utils.h>
#include <RCP_Host/RCP_Host.h>

#include "BaseUI.h"

namespace LRI::RCI {
    class TestControl : public BaseUI {
        static TestControl* instance;

        RCP_TestRunningState_t testState;
        int testNumber;
        bool dataStreaming{};
        bool doHeartbeats{};
        int heartbeatRate{};
        int inputHeartbeatRate{};
        StopWatch heartbeat;

        TestControl();

    public:
        static TestControl* getInstance();

        void render() override;
        void receiveRCPUpdate(const RCP_TestData& data);

        ~TestControl() override = default;
    };
}

#endif //TESTCONTROL_H
