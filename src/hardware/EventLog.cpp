#include "hardware/EventLog.h"

#include <unordered_set>
#include <vector>

#include "utils.h"

namespace LRI::RCI {
    namespace {
        const std::unordered_set READ_ONLY = {
            RCP_DEVCLASS_AM_PRESSURE,
            RCP_DEVCLASS_TEMPERATURE,
            RCP_DEVCLASS_PRESSURE_TRANSDUCER,
            RCP_DEVCLASS_RELATIVE_HYGROMETER,
            RCP_DEVCLASS_LOAD_CELL,
            RCP_DEVCLASS_BOOL_SENSOR,
            RCP_DEVCLASS_POWERMON,
            RCP_DEVCLASS_ACCELEROMETER,
            RCP_DEVCLASS_GYROSCOPE,
            RCP_DEVCLASS_MAGNETOMETER,
            RCP_DEVCLASS_GPS,
        };

        const std::unordered_set f1_DEVS = {RCP_DEVCLASS_ANGLED_ACTUATOR, RCP_DEVCLASS_TEMPERATURE,
                                            RCP_DEVCLASS_PRESSURE_TRANSDUCER, RCP_DEVCLASS_RELATIVE_HYGROMETER,
                                            RCP_DEVCLASS_LOAD_CELL};

        const std::unordered_set f2_DEVS = {RCP_DEVCLASS_STEPPER, RCP_DEVCLASS_POWERMON};

        const std::unordered_set f3_DEVS = {RCP_DEVCLASS_ACCELEROMETER, RCP_DEVCLASS_GYROSCOPE,
                                            RCP_DEVCLASS_MAGNETOMETER};

        const std::unordered_set f4_DEVS = {RCP_DEVCLASS_GPS};

        constexpr size_t DATA_VEC_INIT_SIZE = 5000;
        constexpr size_t ACT_VEC_INIT_SIZE = 25;

        EventLog globalLog;
    } // namespace

    EventLog& EventLog::getGlobalLog() { return globalLog; }

    void EventLog::createDevice(const HardwareQualifier& qual) {
        HardwareChannel ch = {qual, 0};

        if(f1_DEVS.contains(qual.devclass)) target.floats[ch].reserve(DATA_VEC_INIT_SIZE);
        else if(f2_DEVS.contains(qual.devclass)) {
            target.floats[ch].reserve(DATA_VEC_INIT_SIZE);
            ch.channel = 1;
            target.floats[ch].reserve(DATA_VEC_INIT_SIZE);
        }
        else if(f3_DEVS.contains(qual.devclass)) {
            target.floats[ch].reserve(DATA_VEC_INIT_SIZE);
            ch.channel = 1;
            target.floats[ch].reserve(DATA_VEC_INIT_SIZE);
            ch.channel = 2;
            target.floats[ch].reserve(DATA_VEC_INIT_SIZE);
        }
        else if(f4_DEVS.contains(qual.devclass)) {
            target.floats[ch].reserve(DATA_VEC_INIT_SIZE);
            ch.channel = 1;
            target.floats[ch].reserve(DATA_VEC_INIT_SIZE);
            ch.channel = 2;
            target.floats[ch].reserve(DATA_VEC_INIT_SIZE);
            ch.channel = 3;
            target.floats[ch].reserve(DATA_VEC_INIT_SIZE);
        }
        switch(qual.devclass) {
        case RCP_DEVCLASS_SIMPLE_ACTUATOR:
            host.sActWrites[qual.id].reserve(ACT_VEC_INIT_SIZE);
            [[fallthrough]];

        case RCP_DEVCLASS_BOOL_SENSOR:
            target.bools[ch].reserve(DATA_VEC_INIT_SIZE);
            break;

        case RCP_DEVCLASS_ANGLED_ACTUATOR:
            host.aActWrites[qual.id].reserve(ACT_VEC_INIT_SIZE);
            break;

        case RCP_DEVCLASS_STEPPER:
            host.stepperWrites[qual.id].reserve(ACT_VEC_INIT_SIZE);
            break;

        case RCP_DEVCLASS_TEST_STATE:
        case RCP_DEVCLASS_PROMPT:
        case RCP_DEVCLASS_TARGET_LOG:
        case RCP_DEVCLASS_AM_PRESSURE:
        case RCP_DEVCLASS_TEMPERATURE:
        case RCP_DEVCLASS_PRESSURE_TRANSDUCER:
        case RCP_DEVCLASS_RELATIVE_HYGROMETER:
        case RCP_DEVCLASS_LOAD_CELL:
        case RCP_DEVCLASS_POWERMON:
        case RCP_DEVCLASS_ACCELEROMETER:
        case RCP_DEVCLASS_GYROSCOPE:
        case RCP_DEVCLASS_MAGNETOMETER:
        case RCP_DEVCLASS_GPS:
        case RCP_DEVCLASS_AMALGAMATE:
            break;

        default:
#ifdef RCIDEBUG
            throw std::runtime_error("Unaccounted devclass: " + devclassToString(qual.devclass));
#else
            break;
#endif
        }
    }

    void EventLog::clear() {
        target.testRunningState.clear();
        target.misc.clear();
        target.logs.clear();
        target.prompts.clear();
        target.timestamps.clear();
        target.floats.clear();
        target.bools.clear();

        host.testRunningState.clear();
        host.misc.clear();
        host.promptResponses.clear();
        host.readReqs.clear();
        host.aActWrites.clear();
        host.sActWrites.clear();
        host.stepperWrites.clear();
    }

    void EventLog::addTestState(const RCP_TestData& td) {
        target.misc[MiscEvents::DSTREAM].emplace_back(td.timestamp, static_cast<const bool&>(td.dataStreaming));
        target.misc[MiscEvents::INITED].emplace_back(td.timestamp, static_cast<const bool&>(td.isInited));
        target.testRunningState.emplace_back(td.timestamp, td.state);
        if(td.state == RCP_TEST_RUNNING) {
            target.misc[MiscEvents::TEST_START].emplace_back(td.timestamp, td.runningTest);
            target.misc[MiscEvents::TEST_PROG].emplace_back(td.timestamp, td.testProgress);
        }
    }

    void EventLog::addSimpleActuator(const RCP_SimpleActuatorData& sact) {
        HardwareChannel ch = {RCP_DEVCLASS_SIMPLE_ACTUATOR, sact.ID, 0};
        if(!target.bools.contains(ch)) return;
        target.timestamps[ch].emplace_back(sact.timestamp);
        target.bools[ch].emplace_back(sact.state == RCP_SIMPLE_ACTUATOR_ON);
    }

    void EventLog::addBoolData(const RCP_BoolData& bdata) {
        HardwareChannel ch = {RCP_DEVCLASS_BOOL_SENSOR, bdata.ID, 0};
        if(!target.bools.contains(ch)) return;
        target.timestamps[ch].emplace_back(bdata.timestamp);
        target.bools[ch].emplace_back(bdata.data);
    }

    void EventLog::addTargetLog(const RCP_TargetLogData& log) {
        target.logs.emplace_back(log.timestamp, std::string(log.data, log.length));
    }

    void EventLog::addPromptRequest(const RCP_PromptInputRequest& preq) {
        target.prompts.emplace_back(0, PromptRequest{preq.type, std::string(preq.prompt, preq.length)});
    }

    void EventLog::add1F(const RCP_1F& f1) {
        HardwareChannel ch = {f1.devclass, f1.ID, 0};
        if(!target.floats.contains(ch)) return;
        target.floats[ch].emplace_back(f1.data);
        target.timestamps[ch].emplace_back(f1.timestamp);
    }

    void EventLog::add2F(const RCP_2F& f2) {
        HardwareChannel ch = {f2.devclass, f2.ID, 0};
        if(!target.floats.contains(ch)) return;
        target.timestamps[ch].emplace_back(f2.timestamp);
        for(uint8_t channel = 0; channel < 2; channel++) {
            ch.channel = channel;
            target.floats[ch].emplace_back(f2.data[channel]);
        }
    }

    void EventLog::add3F(const RCP_3F& f3) {
        HardwareChannel ch = {f3.devclass, f3.ID, 0};
        if(!target.floats.contains(ch)) return;
        target.timestamps[ch].emplace_back(f3.timestamp);
        for(uint8_t channel = 0; channel < 3; channel++) {
            ch.channel = channel;
            target.floats[ch].emplace_back(f3.data[channel]);
        }
    }

    void EventLog::add4F(const RCP_4F& f4) {
        HardwareChannel ch = {f4.devclass, f4.ID, 0};
        if(!target.floats.contains(ch)) return;
        target.timestamps[ch].emplace_back(f4.timestamp);
        for(uint8_t channel = 0; channel < 4; channel++) {
            ch.channel = channel;
            target.floats[ch].emplace_back(f4.data[channel]);
        }
    }

    void EventLog::addTestStart(uint8_t testnum) {
        auto tp = std::chrono::system_clock::now();
        host.testRunningState.emplace_back(tp, 0, RCP_TEST_RUNNING);
        host.misc[MiscEvents::TEST_START].emplace_back(tp, 0, testnum);
    }

    void EventLog::addTestStop() { host.testRunningState.emplace_back(RCP_TEST_STOPPED); }

    void EventLog::addTestPauseUnpause() { host.testRunningState.emplace_back(RCP_TEST_PAUSED); }

    void EventLog::addHeartbeatSet(uint8_t time) { host.misc[MiscEvents::HEARTBEAT_SET].emplace_back(time); }

    void EventLog::addHeartbeat() { host.misc[MiscEvents::HEARTBEAT].emplace_back(0); }

    void EventLog::addDStreamChange(bool streaming) { host.misc[MiscEvents::DSTREAM].emplace_back(streaming); }

    void EventLog::addESTOP() { host.testRunningState.emplace_back(RCP_TEST_ESTOP); }

    void EventLog::addHWRST() { host.misc[MiscEvents::HWRST].emplace_back(0); }

    void EventLog::addTMRST() { host.misc[MiscEvents::TMRST].emplace_back(0); }

    void EventLog::addPromptResponse(float val) {
        host.promptResponses.emplace_back(PromptResponse{.index = target.prompts.size() - 1, .fdata = val});
    }

    void EventLog::addPromptResponse(bool val) {
        host.promptResponses.emplace_back(PromptResponse{.index = target.prompts.size() - 1, .bdata = val});
    }

    void EventLog::addAActWrite(uint8_t id, float val) {
        if(!host.aActWrites.contains(id)) return;
        host.aActWrites[id].emplace_back(val);
    }

    void EventLog::addStepperWrite(uint8_t id, RCP_StepperControlMode mode, float val) {
        if(!host.stepperWrites.contains(id)) return;
        host.stepperWrites[id].emplace_back(StepperWrite{mode, val});
    }

    void EventLog::addSActWrite(uint8_t id, RCP_SimpleActuatorState val) {
        if(!host.sActWrites.contains(id)) return;
        host.sActWrites[id].emplace_back(val == RCP_SIMPLE_ACTUATOR_ON);
    }

    void EventLog::addReadReq(const HardwareQualifier& qual) { host.readReqs.emplace_back(qual); }

    void EventLog::addReceived(const void* data, size_t length) {
        const uint8_t* udata = static_cast<const uint8_t*>(data);
        receivedBytes.insert(receivedBytes.end(), udata, udata + length);
    }

    void EventLog::addSent(const void* data, size_t length) {
        const uint8_t* udata = static_cast<const uint8_t*>(data);
        sentBytes.insert(sentBytes.end(), udata, udata + length);
    }
} // namespace LRI::RCI
