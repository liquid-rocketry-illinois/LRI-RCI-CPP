#ifndef TESTSTATE_H
#define TESTSTATE_H

#include "RCP_Host/RCP_Host.h"

#include "utils.h"

namespace LRI::RCI {
    class TestState {
        RCP_TestRunningState state;
        uint8_t testNum;
        uint8_t heartbeatTime;
        bool dataStreaming;

        StopWatch heartbeat;

        TestState() = default;
        ~TestState() = default;

    public:
        static TestState* getInstance();

        [[nodiscard]] int getTestNum() const;
        [[nodiscard]] RCP_TestRunningState getState() const;
        [[nodiscard]] uint8_t getHeartbeatTime() const;
        [[nodiscard]] bool getDataStreaming() const;

        bool startTest(uint8_t number);
        bool stopTest();
        bool pause();
        bool setHeartbeatTime(uint8_t time);
        bool setDataStreaming(bool stream);

        void update();

        void receiveRCPUpdate(const RCP_TestData& testState);
    };
}

#endif //TESTSTATE_H
