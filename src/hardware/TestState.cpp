#include "hardware/TestState.h"

#include "hardware/Sensors.h"

// Hardware singleton representing test state. Most of these functions are self explanatory
namespace LRI::RCI {
    std::atomic_bool TestState::inited = false;

    TestState* TestState::getInstance() {
        static TestState* instance = nullptr;
        if(instance == nullptr) instance = new TestState();
        return instance;
    }

    bool TestState::getInited() { return inited.load(); }

    bool TestState::startTest(uint8_t number) {
        if(state != RCP_TEST_STOPPED) return false;
        state = RCP_TEST_RUNNING;
        RCP_startTest(number);
        if(resetTimeOnStart) {
            RCP_deviceTimeReset();
            Sensors::getInstance()->clearAll();
        }

        activeTest = number;
        return true;
    }

    bool TestState::stopTest() {
        if(state == RCP_TEST_STOPPED || state == RCP_TEST_ESTOP) return false;
        RCP_stopTest();
        state = RCP_TEST_STOPPED;
        return true;
    }

    bool TestState::pause() {
        if(state == RCP_TEST_ESTOP || state == RCP_TEST_STOPPED) return false;
        RCP_pauseUnpauseTest();
        state = (state == RCP_TEST_RUNNING) ? RCP_TEST_PAUSED : RCP_TEST_RUNNING;
        return true;
    }

    uint8_t TestState::getActiveTest() const { return activeTest; }

    void TestState::setTests(const std::map<uint8_t, std::string>& testlist) { tests = testlist; }

    const std::map<uint8_t, std::string>* TestState::getTestOptions() const { return &tests; }


    RCP_TestRunningState TestState::getState() const { return state; }

    uint8_t TestState::getHeartbeatTime() const { return heartbeatTime; }

    bool TestState::setHeartbeatTime(uint8_t time) {
        if(time > 14) return false;
        bool complete = !RCP_setHeartbeatTime(time);
        if(complete) heartbeatTime = time;
        return complete;
    }

    bool TestState::getDataStreaming() const { return dataStreaming; }

    bool TestState::setDataStreaming(bool stream) {
        bool complete = !RCP_setDataStreaming(stream);
        if(complete) dataStreaming = stream;
        return complete;
    }

    void TestState::setResetTimeOnTestStart(bool reset) { resetTimeOnStart = reset; }

    bool TestState::deviceReset() { return !RCP_deviceReset(); }

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
} // namespace LRI::RCI
