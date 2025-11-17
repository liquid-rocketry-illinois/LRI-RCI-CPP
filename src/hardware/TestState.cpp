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

    bool startTest(uint8_t number) {
        if(state != RCP_TEST_STOPPED) return false;
        state = RCP_TEST_RUNNING;
        RCP_startTest(number);
        if(resetTimeOnStart) {
            RCP_deviceTimeReset();
            Sensors::clearAll();
        }

        activeTest = number;
        return true;
    }

    bool stopTest() {
        if(state == RCP_TEST_STOPPED || state == RCP_TEST_ESTOP) return false;
        RCP_stopTest();
        state = RCP_TEST_STOPPED;
        return true;
    }

    bool pause() {
        if(state == RCP_TEST_ESTOP || state == RCP_TEST_STOPPED) return false;
        RCP_pauseUnpauseTest();
        state = (state == RCP_TEST_RUNNING) ? RCP_TEST_PAUSED : RCP_TEST_RUNNING;
        return true;
    }

    uint8_t getActiveTest() { return activeTest; }

    void setTests(const std::map<uint8_t, std::string>& testlist) { tests = testlist; }

    const std::map<uint8_t, std::string>* getTestOptions() { return &tests; }


    RCP_TestRunningState getState() { return state; }

    uint8_t getHeartbeatTime() { return heartbeatTime; }

    bool setHeartbeatTime(uint8_t time) {
        if(time > 14) return false;
        bool complete = !RCP_setHeartbeatTime(time);
        if(complete) heartbeatTime = time;
        return complete;
    }

    bool getDataStreaming() { return dataStreaming; }

    bool setDataStreaming(bool stream) {
        bool complete = !RCP_setDataStreaming(stream);
        if(complete) dataStreaming = stream;
        return complete;
    }

    void setResetTimeOnTestStart(bool reset) { resetTimeOnStart = reset; }

    bool deviceReset() { return !RCP_deviceReset(); }

    void update() {
        if(heartbeatTime == 0) return;
        if(heartbeat.timeSince() > heartbeatTime) {
            if(!RCP_sendHeartbeat()) {
                heartbeat.reset();
            }
        }
    }

    void reset() { inited = false; }

    int receiveRCPUpdate(RCP_TestData data) {
        heartbeatTime = data.heartbeatTime;
        dataStreaming = data.dataStreaming;
        state = data.state;
        inited = data.isInited;
        return 0;
    }
} // namespace LRI::RCI::TestState
