#include "hardware/TestState.h"

namespace LRI::RCI {
    std::atomic_bool TestState::inited = false;

    TestState* TestState::getInstance() {
        static TestState* instance = nullptr;
        if(instance == nullptr) instance = new TestState();
        return instance;
    }

    bool TestState::getInited() {
        return inited.load();
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
        RCP_stopTest();
        state = RCP_TEST_STOPPED;
        return true;
    }

    bool TestState::pause() {
        if(state != RCP_TEST_PAUSED && state != RCP_TEST_RUNNING) return false;
        RCP_pauseUnpauseTest();
        state = (state == RCP_TEST_RUNNING) ? RCP_TEST_PAUSED : RCP_TEST_RUNNING;
        return true;
    }

    int TestState::getTestNum() const {
        return testNum;
    }

    RCP_TestRunningState TestState::getState() const {
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

    void TestState::update() {
        if(heartbeatTime == 0) return;
        if(heartbeat.timeSince() > heartbeatTime) {
            if(!RCP_sendHeartbeat()) {
                heartbeat.reset();
            }
        }
    }

    void TestState::receiveRCPUpdate(const RCP_TestData& testState) {
        heartbeatTime = testState.heartbeatTime;
        dataStreaming = testState.dataStreaming;
        state = testState.state;
        inited = testState.isInited;
    }
}
