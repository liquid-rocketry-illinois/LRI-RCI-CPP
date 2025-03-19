#ifndef TESTSTATE_H
#define TESTSTATE_H

#include "RCP_Host/RCP_Host.h"

#include "utils.h"
// NOLINTBEGIN
namespace LRI::RCI {
    class TestState { // For some reason clang tidy warns about the fields below not being initialized by
                      // the constructor even though its a defualt constructor, hence the NOLINTing
        RCP_TestRunningState_t state; // NOLINTEND
        uint8_t testNum;
        uint8_t heartbeatTime;
        bool dataStreaming;

        StopWatch heartbeat;

        TestState() = default;
        ~TestState() = default;

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

        void update();

        void receiveRCPUpdate(const RCP_TestData& testState);
    };
}

#endif //TESTSTATE_H
