#include "hardware/EventLog.h"

#include <vector>

#include "utils.h"

namespace LRI::RCI {
    namespace {
        constexpr size_t DATA_VEC_INIT_SIZE = 5000;
        constexpr size_t ACT_VEC_INIT_SIZE = 25;

        const HardwareChannel TEST_STATE{RCP_DEVCLASS_TEST_STATE, 0, 0};
        const HardwareQualifier TARGET_LOG{RCP_DEVCLASS_TARGET_LOG, 0};
        const HardwareChannel PROMPT{RCP_DEVCLASS_PROMPT, 0, 0};
    } // namespace

    namespace TestStateChannels {
        // Oh man if only there was some kind of reflective introspection system...
        constexpr Channels ALL_TCH[] = {
            T_INITED, T_DSTREAM, T_HEARTBEAT_TIME, T_TEST_RUN_STATE, T_RUNNING_TEST, T_TEST_PROGRESS,
        };

        constexpr Channels ALL_HCH[] = {
            H_HEARTBEAT, H_DSTREAM, H_HEARTBEAT_TIME, H_TEST_RUN_STATE, H_TEST_ID, H_TIME_RESET, H_DEV_RESET,
        };

        const HardwareChannel HEARTBEAT{TEST_STATE, H_HEARTBEAT};
        const HardwareChannel DSTREAM{TEST_STATE, H_DSTREAM};
        const HardwareChannel HBTIME{TEST_STATE, H_HEARTBEAT_TIME};
        const HardwareChannel RUNSTATE{TEST_STATE, H_TEST_RUN_STATE};
        const HardwareChannel TESTID{TEST_STATE, H_TEST_ID};
        const HardwareChannel TIMERST{TEST_STATE, H_TIME_RESET};
        const HardwareChannel DEVRST{TEST_STATE, H_DEV_RESET};

        const std::map<Channels, uint8_t (*)(const RCP_TestData&)> CHANNEL_GETTERS = {
            {T_INITED, [](const RCP_TestData& d) -> uint8_t { return d.isInited; }},
            {T_DSTREAM, [](const RCP_TestData& d) -> uint8_t { return d.dataStreaming; }},
            {T_HEARTBEAT_TIME, [](const RCP_TestData& d) -> uint8_t { return d.heartbeatTime; }},
            {T_TEST_RUN_STATE, [](const RCP_TestData& d) -> uint8_t { return d.state; }},
            {T_RUNNING_TEST, [](const RCP_TestData& d) -> uint8_t { return d.runningTest; }},
            {T_TEST_PROGRESS, [](const RCP_TestData& d) -> uint8_t { return d.testProgress; }},
        };
    } // namespace TestStateChannels

    EventLog::EventLog() {
        HardwareChannel ch{TEST_STATE};

        target.timestamps[TEST_STATE].reserve(DATA_VEC_INIT_SIZE);

        for(const auto c : TestStateChannels::ALL_TCH) {
            ch.channel = c;
            target.uints[ch].reserve(DATA_VEC_INIT_SIZE);
        }

        for(const auto c : TestStateChannels::ALL_HCH) {
            ch.channel = c;

            // The test ID to run can be matched up to entries in the test state array, so it does not need seperate
            // timestamps
            if(c != TestStateChannels::H_TEST_ID) host.ctrltimestamps[ch].reserve(ACT_VEC_INIT_SIZE);

            // None of these have associated metadata
            if(c == TestStateChannels::H_HEARTBEAT || c == TestStateChannels::H_DEV_RESET ||
               c == TestStateChannels::H_TIME_RESET)
                continue;
            host.ctrl_uints[ch].reserve(ACT_VEC_INIT_SIZE);
        }

        host.acttimestamps[TARGET_LOG].reserve(ACT_VEC_INIT_SIZE);
        host.strings[TARGET_LOG].reserve(ACT_VEC_INIT_SIZE);

        // To store prompt request string
        host.acttimestamps[PROMPT].reserve(ACT_VEC_INIT_SIZE);
        host.strings[PROMPT].reserve(ACT_VEC_INIT_SIZE);
        // To store prompt type
        host.ctrl_uints[PROMPT].reserve(ACT_VEC_INIT_SIZE);

        // To store response information
        host.ctrltimestamps[PROMPT].reserve(ACT_VEC_INIT_SIZE);

        // To store responses themselves
        host.act_uints[PROMPT].reserve(ACT_VEC_INIT_SIZE);
        host.act_floats[PROMPT].reserve(ACT_VEC_INIT_SIZE);
    }

    void EventLog::createDevice(const HardwareQualifier& qual) {
        HardwareChannel ch{qual, 0};

        switch(qual.devclass) {
        case RCP_DEVCLASS_SIMPLE_ACTUATOR:
        case RCP_DEVCLASS_DISCRETE_ACTUATOR: {
            // For data sent back
            target.timestamps[qual].reserve(DATA_VEC_INIT_SIZE);
            target.uints[ch].reserve(DATA_VEC_INIT_SIZE);

            // For the control data
            host.acttimestamps[qual].reserve(ACT_VEC_INIT_SIZE);
            host.act_uints[qual].reserve(ACT_VEC_INIT_SIZE);
            break;
        }

        case RCP_DEVCLASS_STEPPER: {
            // For the 2 floats sent back
            target.timestamps[qual].reserve(DATA_VEC_INIT_SIZE);
            target.floats[ch].reserve(DATA_VEC_INIT_SIZE);
            ch.channel = 1;
            target.floats[ch].reserve(DATA_VEC_INIT_SIZE);

            // For the control mode and value sent as writes
            host.acttimestamps[qual].reserve(ACT_VEC_INIT_SIZE);
            host.act_floats[qual].reserve(ACT_VEC_INIT_SIZE);
            host.act_uints[qual].reserve(ACT_VEC_INIT_SIZE);
            break;
        }

        case RCP_DEVCLASS_ANGLED_ACTUATOR:
        case RCP_DEVCLASS_MOTOR: {
            // For data sent back
            target.timestamps[qual].reserve(DATA_VEC_INIT_SIZE);
            target.floats[ch].reserve(DATA_VEC_INIT_SIZE);

            // For the control data
            host.acttimestamps[qual].reserve(ACT_VEC_INIT_SIZE);
            host.act_floats[qual].reserve(ACT_VEC_INIT_SIZE);
            break;
        }

        case RCP_DEVCLASS_AM_PRESSURE:
        case RCP_DEVCLASS_TEMPERATURE:
        case RCP_DEVCLASS_PRESSURE_TRANSDUCER:
        case RCP_DEVCLASS_RELATIVE_HYGROMETER:
        case RCP_DEVCLASS_LOAD_CELL:
        case RCP_DEVCLASS_FLOW_METER:
        case RCP_DEVCLASS_ALTITUDE:
        case RCP_DEVCLASS_RADIO_STRENGTH: {
            // For data sent back
            target.timestamps[qual].reserve(DATA_VEC_INIT_SIZE);
            target.floats[ch].reserve(DATA_VEC_INIT_SIZE);

            host.ctrltimestamps[ch].reserve(ACT_VEC_INIT_SIZE);
            host.tares[ch].reserve(ACT_VEC_INIT_SIZE);
        }

        case RCP_DEVCLASS_BOOL_SENSOR: {
            target.timestamps[qual].reserve(DATA_VEC_INIT_SIZE);
            target.uints[ch].reserve(DATA_VEC_INIT_SIZE);

            host.ctrltimestamps[ch].reserve(ACT_VEC_INIT_SIZE);
            host.tares[ch].reserve(ACT_VEC_INIT_SIZE);
            break;
        }

        case RCP_DEVCLASS_POWERMON: {
            target.timestamps[qual].reserve(DATA_VEC_INIT_SIZE);

            target.floats[ch].reserve(DATA_VEC_INIT_SIZE);
            host.ctrltimestamps[ch].reserve(ACT_VEC_INIT_SIZE);
            host.tares[ch].reserve(ACT_VEC_INIT_SIZE);

            ch.channel = 1;
            target.floats[ch].reserve(DATA_VEC_INIT_SIZE);
            host.ctrltimestamps[ch].reserve(ACT_VEC_INIT_SIZE);
            host.tares[ch].reserve(ACT_VEC_INIT_SIZE);

            break;
        }

        case RCP_DEVCLASS_ACCELEROMETER:
        case RCP_DEVCLASS_GYROSCOPE:
        case RCP_DEVCLASS_MAGNETOMETER:
        case RCP_DEVCLASS_RPY: {
            target.timestamps[qual].reserve(DATA_VEC_INIT_SIZE);

            target.floats[ch].reserve(DATA_VEC_INIT_SIZE);
            host.ctrltimestamps[ch].reserve(ACT_VEC_INIT_SIZE);
            host.tares[ch].reserve(ACT_VEC_INIT_SIZE);

            ch.channel = 1;
            target.floats[ch].reserve(DATA_VEC_INIT_SIZE);
            host.ctrltimestamps[ch].reserve(ACT_VEC_INIT_SIZE);
            host.tares[ch].reserve(ACT_VEC_INIT_SIZE);

            ch.channel = 2;
            target.floats[ch].reserve(DATA_VEC_INIT_SIZE);
            host.ctrltimestamps[ch].reserve(ACT_VEC_INIT_SIZE);
            host.tares[ch].reserve(ACT_VEC_INIT_SIZE);

            break;
        }

        case RCP_DEVCLASS_GPS:
        case RCP_DEVCLASS_QUATERNION: {
            target.timestamps[qual].reserve(DATA_VEC_INIT_SIZE);

            target.floats[ch].reserve(DATA_VEC_INIT_SIZE);
            host.ctrltimestamps[ch].reserve(ACT_VEC_INIT_SIZE);
            host.tares[ch].reserve(ACT_VEC_INIT_SIZE);

            ch.channel = 1;
            target.floats[ch].reserve(DATA_VEC_INIT_SIZE);
            host.ctrltimestamps[ch].reserve(ACT_VEC_INIT_SIZE);
            host.tares[ch].reserve(ACT_VEC_INIT_SIZE);

            ch.channel = 2;
            target.floats[ch].reserve(DATA_VEC_INIT_SIZE);
            host.ctrltimestamps[ch].reserve(ACT_VEC_INIT_SIZE);
            host.tares[ch].reserve(ACT_VEC_INIT_SIZE);

            ch.channel = 3;
            target.floats[ch].reserve(DATA_VEC_INIT_SIZE);
            host.ctrltimestamps[ch].reserve(ACT_VEC_INIT_SIZE);
            host.tares[ch].reserve(ACT_VEC_INIT_SIZE);

            break;
        }

        default: {}
            // Amalgamate is not valid here
            // Test state, target log, and prompts are already handled
        }
    }

    void EventLog::clear() {
        target.timestamps.clear();
        target.floats.clear();
        target.uints.clear();

        host.acttimestamps.clear();
        host.strings.clear();
        host.act_uints.clear();
        host.act_floats.clear();
        host.ctrltimestamps.clear();
        host.tares.clear();
        host.ctrl_uints.clear();
        host.readReqs.clear();
    }

    void EventLog::addTestState(const RCP_TestData& td) {
        target.timestamps[TEST_STATE].emplace_back(td.timestamp);
        HardwareChannel ch{TEST_STATE, 0};

        for(const auto& c : TestStateChannels::ALL_TCH) {
            ch.channel = c;
            target.uints[ch].emplace_back(TestStateChannels::CHANNEL_GETTERS.at(c)(td));
        }
    }

    void EventLog::addPromptRequest(const RCP_PromptInputRequest& preq) {
        host.acttimestamps[PROMPT].emplace_back();
        if(preq.type == RCP_PromptDataType_RESET) host.strings[PROMPT].emplace_back("RESET");
        else host.strings[PROMPT].emplace_back(preq.prompt, preq.length);
    }

    void EventLog::addTargetLog(const RCP_TargetLogData& log) {
        host.acttimestamps[TARGET_LOG].emplace_back();
        host.strings[TARGET_LOG].emplace_back(log.data, log.length);
    }

    void EventLog::addByteData(const RCP_ByteData& data) {
        HardwareChannel ch = {data.devclass, data.ID, 0};
        if(!target.uints.contains(ch)) return;
        target.uints[ch].emplace_back(data.data);
        target.timestamps[ch].emplace_back(data.timestamp);
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
        for(; ch.channel < 2; ch.channel++) target.floats[ch].emplace_back(f2.data[ch.channel]);
    }

    void EventLog::add3F(const RCP_3F& f3) {
        HardwareChannel ch = {f3.devclass, f3.ID, 0};
        if(!target.floats.contains(ch)) return;
        target.timestamps[ch].emplace_back(f3.timestamp);
        for(; ch.channel < 3; ch.channel++) target.floats[ch].emplace_back(f3.data[ch.channel]);
    }

    void EventLog::add4F(const RCP_4F& f4) {
        HardwareChannel ch = {f4.devclass, f4.ID, 0};
        if(!target.floats.contains(ch)) return;
        target.timestamps[ch].emplace_back(f4.timestamp);
        for(; ch.channel < 4; ch.channel++) target.floats[ch].emplace_back(f4.data[ch.channel]);
    }

    using namespace TestStateChannels;

    void EventLog::addTestStart(uint8_t testnum) {
        host.ctrltimestamps[RUNSTATE].emplace_back();
        host.ctrl_uints[RUNSTATE].emplace_back(RCP_TEST_START);
        host.ctrl_uints[TESTID].emplace_back(testnum);
    }

    void EventLog::addTestStop() {
        host.ctrltimestamps[RUNSTATE].emplace_back();
        host.ctrl_uints[RUNSTATE].emplace_back(RCP_TEST_STOP);
    }

    void EventLog::addTestPauseUnpause() {
        host.ctrltimestamps[RUNSTATE].emplace_back();
        host.ctrl_uints[RUNSTATE].emplace_back(RCP_TEST_PAUSE);
    }

    void EventLog::addHeartbeatSet(uint8_t time) {
        host.ctrltimestamps[HBTIME].emplace_back();
        host.ctrl_uints[HBTIME].emplace_back(time);
    }

    void EventLog::addHeartbeat() { host.ctrltimestamps[HEARTBEAT].emplace_back(); }

    void EventLog::addDStreamChange(bool streaming) {
        host.ctrltimestamps[DSTREAM].emplace_back();
        host.ctrl_uints[DSTREAM].emplace_back(streaming ? 1 : 0);
    }

    void EventLog::addESTOP() {
        host.ctrltimestamps[RUNSTATE].emplace_back();
        host.ctrl_uints[RUNSTATE].emplace_back(RCP_TEST_ESTOP);
    }

    void EventLog::addHWRST() { host.ctrltimestamps[DEVRST].emplace_back(); }

    void EventLog::addTMRST() { host.ctrltimestamps[TIMERST].emplace_back(); }

    void EventLog::addPromptResponse(float val) {
        host.ctrltimestamps[PROMPT].emplace_back();
        host.act_uints[PROMPT].emplace_back(0);
        host.act_floats[PROMPT].emplace_back(val);
    }

    void EventLog::addPromptResponse(bool val) {
        host.ctrltimestamps[PROMPT].emplace_back();
        host.act_uints[PROMPT].emplace_back(val);
        host.act_floats[PROMPT].emplace_back(0.0f);
    }

    void EventLog::addAActWrite(uint8_t id, float val) {
        HardwareQualifier qual{RCP_DEVCLASS_ANGLED_ACTUATOR, id};
        if(!host.act_floats.contains(qual)) return;
        host.acttimestamps[qual].emplace_back();
        host.act_floats[qual].emplace_back(val);
    }

    void EventLog::addMotorWrite(uint8_t id, float val) {
        HardwareQualifier qual{RCP_DEVCLASS_MOTOR, id};
        if(!host.act_floats.contains(qual)) return;
        host.acttimestamps[qual].emplace_back();
        host.act_floats[qual].emplace_back(val);
    }

    void EventLog::addStepperWrite(uint8_t id, RCP_StepperControlMode mode, float val) {
        HardwareQualifier qual{RCP_DEVCLASS_STEPPER, id};
        if(!host.act_floats.contains(qual)) return;
        host.acttimestamps[qual].emplace_back();
        host.act_floats[qual].emplace_back(val);
        host.act_uints[qual].emplace_back(mode);
    }

    void EventLog::addSActWrite(uint8_t id, RCP_SimpleActuatorState val) {
        HardwareQualifier qual{RCP_DEVCLASS_SIMPLE_ACTUATOR, id};
        if(!host.act_uints.contains(qual)) return;
        host.acttimestamps[qual].emplace_back();
        host.act_uints[qual].emplace_back(val);
    }

    void EventLog::addDActWrite(uint8_t id, uint8_t val) {
        HardwareQualifier qual{RCP_DEVCLASS_DISCRETE_ACTUATOR, id};
        if(!host.act_uints.contains(qual)) return;
        host.acttimestamps[qual].emplace_back();
        host.act_uints[qual].emplace_back(val);
    }

    void EventLog::addReadReq(const HardwareQualifier& qual) { host.readReqs.emplace_back(getHostTime(), qual); }

    void EventLog::addTare(const HardwareChannel& ch, float off) {
        if(!host.tares.contains(ch)) return;
        host.ctrltimestamps[ch].emplace_back();
        host.tares[ch].emplace_back(off);
    }

    void EventLog::addReceived(const void* data, size_t length) {
        const uint8_t* udata = static_cast<const uint8_t*>(data);
        rxbytes.insert(rxbytes.end(), udata, udata + length);
    }

    void EventLog::addSent(const void* data, size_t length) {
        const uint8_t* udata = static_cast<const uint8_t*>(data);
        txbytes.insert(txbytes.end(), udata, udata + length);
    }

    // Getters
    TargetUint EventLog::getReportedTestStateChannel(Channels ch) {
        HardwareChannel channel{TEST_STATE, ch};
        if(!target.uints.contains(channel)) return {};
        return std::make_tuple(&target.timestamps.at(TEST_STATE), &target.uints.at(channel));
    }

    HostString EventLog::getPromptRequests() const {
        return std::make_tuple(&host.acttimestamps.at(PROMPT), &host.strings.at(PROMPT));
    }

    PromptResponse EventLog::getPromptResponses() const {
        return std::make_tuple(&host.ctrltimestamps.at(PROMPT), &host.act_floats.at(PROMPT), &host.act_uints.at(PROMPT));
    }

    HostString EventLog::getLogs() const {
        return std::make_tuple(&host.acttimestamps.at(TARGET_LOG), &host.strings.at(TARGET_LOG));
    }

    TargetUint EventLog::getChannelUintData(const HardwareChannel& ch) const {
        if(!target.uints.contains(ch)) return {};
        return std::make_tuple(&target.timestamps.at(ch), &target.uints.at(ch));
    }

    TargetFloat EventLog::getChannelFloatData(const HardwareChannel& ch) const {
        if(!target.floats.contains(ch)) return {};
        return std::make_tuple(&target.timestamps.at(ch), &target.floats.at(ch));
    }

    HostUint EventLog::getRequestedRunningStates() const {
        return std::make_tuple(&host.ctrltimestamps.at(TEST_STATE), &host.ctrl_uints.at(TEST_STATE));
    }

    const UintList* EventLog::getRequestedTestStartIDs() const { return &host.ctrl_uints.at(TESTID); }

    HostUint EventLog::getRequestedHeartbeatTimeSet() const {
        return std::make_tuple(&host.ctrltimestamps.at(HBTIME), &host.ctrl_uints.at(HBTIME));
    }

    const TimePointList* EventLog::getHeartbeats() const { return &host.ctrltimestamps.at(HEARTBEAT); }

    HostUint EventLog::getRequestedDStreams() const {
        return std::make_tuple(&host.ctrltimestamps.at(DSTREAM), &host.ctrl_uints.at(DSTREAM));
    }

    const TimePointList* EventLog::getRequestedHWResets() const { return &host.ctrltimestamps.at(DEVRST); }
    const TimePointList* EventLog::getRequestedTimeResets() const { return &host.ctrltimestamps.at(TIMERST); }

    HostFloat EventLog::getRequestedAActWrites(uint8_t id) const {
        HardwareQualifier qual{RCP_DEVCLASS_ANGLED_ACTUATOR, id};
        if(!host.act_floats.contains(qual)) return {};
        return std::make_tuple(&host.acttimestamps.at(qual), &host.act_floats.at(qual));
    }

    HostFloat EventLog::getRequestedMotorWrites(uint8_t id) const {
        HardwareQualifier qual{RCP_DEVCLASS_MOTOR, id};
        if(!host.act_floats.contains(qual)) return {};
        return std::make_tuple(&host.acttimestamps.at(qual), &host.act_floats.at(qual));
    }

    StepperWritesList EventLog::getRequestedStepperWrites(uint8_t id) const {
        HardwareQualifier qual{RCP_DEVCLASS_STEPPER, id};
        if(!host.act_uints.contains(qual)) return {};
        return std::make_tuple(&host.acttimestamps.at(qual), &host.act_uints.at(qual), &host.act_floats.at(qual));
    }

    HostUint EventLog::getRequestedSActWrites(uint8_t id) const {
        HardwareQualifier qual{RCP_DEVCLASS_SIMPLE_ACTUATOR, id};
        if(!host.act_uints.contains(qual)) return {};
        return std::make_tuple(&host.acttimestamps.at(qual), &host.act_uints.at(qual));
    }

    HostUint EventLog::getRequestedDActWrites(uint8_t id) const {
        HardwareQualifier qual{RCP_DEVCLASS_DISCRETE_ACTUATOR, id};
        if(!host.act_uints.contains(qual)) return {};
        return std::make_tuple(&host.acttimestamps.at(qual), &host.act_uints.at(qual));
    }

    HostFloat EventLog::getRequestedTares(const HardwareChannel& ch) const {
        if(!host.tares.contains(ch)) return {};
        return std::make_tuple(&host.ctrltimestamps.at(ch), &host.tares.at(ch));
    }

    const ReadRequestsList* EventLog::getReadRequests() const { return &host.readReqs; }

    const UintList* EventLog::getRXBytes() const { return &rxbytes; }
    const UintList* EventLog::getTXBytes() const { return &txbytes; }
} // namespace LRI::RCI
