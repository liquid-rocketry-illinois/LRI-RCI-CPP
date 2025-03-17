#include "hardware/TestState.h"

namespace LRI::RCI {
    TestState* TestState::getInstance() {
        static TestState* instance = nullptr;
        if(instance == nullptr) instance = new TestState();
        return instance;
    }

    bool TestState::startTest(uint8_t number) {
        if(state != RCP_TEST_STOPPED) return false;
        testNum = number;
        state = RCP_TEST_RUNNING;
        RCP_startTest(testNum);
        return true;
    }

    bool TestState::stopTest() {
        if(state != RCP_TEST_STOPPED && state != RCP_TEST_ESTOP) return false;
        // TODO: fill in with rcp method once added
        state = RCP_TEST_STOPPED;
        return true;
    }

    bool TestState::pause() {
        if(state != RCP_TEST_PAUSED && state != RCP_TEST_RUNNING) return false;
        // TODO: fill in with rcp method once added
        state = (state == RCP_TEST_RUNNING) ? RCP_TEST_PAUSED : RCP_TEST_RUNNING;
        return true;
    }

    int TestState::getTestNum() const {
        return testNum;
    }

    RCP_TestRunningState_t TestState::getState() const {
        return state;
    }

    uint8_t TestState::getHeartbeatTime() const {
        return heartbeatTime;
    }

    bool TestState::setHeartbeatTime(uint8_t time) {
        if(time > 14) return false;
        bool complete = !RCP_setHeartbeatTime(time);
        if(complete) heartbeatTime = time;
        return complete;
    }

    bool TestState::getDataStreaming() const {
        return dataStreaming;
    }

    bool TestState::setDataStreaming(bool stream) {
        bool complete = !RCP_setDataStreaming(stream);
        if(complete) dataStreaming = stream;
        return complete;
    }

    void TestState::receiveRCPUpdate(const RCP_TestData& testState) {
        heartbeatTime = testState.heartbeatTime;
        dataStreaming = testState.dataStreaming;
        state = testState.state;
    }
}