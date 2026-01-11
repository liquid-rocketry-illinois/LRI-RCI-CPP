#include "hardware/TestState.h"

#include "hardware/Sensors.h"

// Hardware singleton representing test state. Most of these functions are self explanatory
namespace LRI::RCI::TestState {
    // A global variable for indicating if the target has sent an init packet. This is an atomic so that
    // the interfaces can determine if they can start sending data from the buffers
    static std::atomic_bool inited;

    // Current test state storage
    static RCP_TestRunningState state;
    static std::map<uint8_t, std::string> tests;
    static uint8_t activeTest;
    static uint8_t heartbeatTime;
    static bool dataStreaming;
    static bool resetTimeOnStart;

    // Timer for heartbeats
    static StopWatch heartbeat;

    bool getInited() { return inited.load(); }

    void setTests(const std::map<uint8_t, std::string>& testlist) { tests = testlist; }

    // A bunch of one line getters
    const std::map<uint8_t, std::string>* getTestOptions() { return &tests; }
    uint8_t getActiveTest() { return activeTest; }
    RCP_TestRunningState getState() { return state; }
    uint8_t getHeartbeatTime() { return heartbeatTime; }
    bool getDataStreaming() { return dataStreaming; }

    void startTest(uint8_t number) {
        RCP_startTest(number);
        if(resetTimeOnStart) {
            RCP_deviceTimeReset();
            EventLog::getGlobalLog().addTMRST();
        }

        activeTest = number;
        EventLog::getGlobalLog().addTestStart(number);
    }

    void stopTest() {
        RCP_stopTest();
        state = RCP_TEST_STOPPED;
        EventLog::getGlobalLog().addTestStop();
    }

    void pause() {
        RCP_pauseUnpauseTest();
        state = (state == RCP_TEST_RUNNING) ? RCP_TEST_PAUSED : RCP_TEST_RUNNING;
        EventLog::getGlobalLog().addTestPauseUnpause();
    }

    void setHeartbeatTime(uint8_t time) {
        RCP_setHeartbeatTime(time);
        EventLog::getGlobalLog().addHeartbeatSet(time);
    }

    void setDataStreaming(bool stream) {
        RCP_setDataStreaming(stream);
        EventLog::getGlobalLog().addDStreamChange(stream);
    }

    void setResetTimeOnTestStart(bool reset) { resetTimeOnStart = reset; }

    void deviceReset() {
        EventLog::getGlobalLog().addHWRST();
        RCP_deviceReset();
    }

    void ESTOP() {
        EventLog::getGlobalLog().addESTOP();
        RCP_sendEStop();
    }

    void update() {
        if(heartbeatTime == 0) return;
        if(heartbeat.timeSince() > static_cast<float>(heartbeatTime)) {
            if(!RCP_sendHeartbeat()) heartbeat.reset();
            EventLog::getGlobalLog().addHeartbeat();
        }
    }

    void reset() {
        inited = false;
        state = RCP_TEST_STOPPED;
        tests.clear();
        dataStreaming = false;
        resetTimeOnStart = false;
    }

    RCP_Error receiveRCPUpdate(RCP_TestData data) {
        heartbeatTime = data.heartbeatTime;
        dataStreaming = data.dataStreaming;
        state = data.state;
        inited = data.isInited;
        EventLog::getGlobalLog().addTestState(data);
        return RCP_ERR_SUCCESS;
    }
} // namespace LRI::RCI::TestState
