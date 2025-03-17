#ifndef TESTSTATE_H
#define TESTSTATE_H

#include "RCP_Host/RCP_Host.h"

namespace LRI::RCI {
    class TestState {
        TestState() = default;
        ~TestState() = default;

        RCP_TestRunningState_t state;
        uint8_t testNum;
        uint8_t heartbeatTime;
        bool dataStreaming;

    public:
        static TestState* getInstance();

        [[nodiscard]] int getTestNum() const;
        [[nodiscard]] RCP_TestRunningState_t getState() const;
        [[nodiscard]] uint8_t getHeartbeatTime() const;
        [[nodiscard]] bool getDataStreaming() const;

        bool startTest(uint8_t number);
        bool stopTest();
        bool pause();
        bool setHeartbeatTime(uint8_t time);
        bool setDataStreaming(bool stream);

        void receiveRCPUpdate(const RCP_TestData& testState);
    };
}

#endif //TESTSTATE_H
