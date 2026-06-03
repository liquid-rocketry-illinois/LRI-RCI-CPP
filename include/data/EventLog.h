#ifndef LRI_CONTROL_PANEL_EVENTLOG_H
#define LRI_CONTROL_PANEL_EVENTLOG_H

#include <chrono>
#include <map>
#include <string>
#include <vector>

#include "HardwareQualifier.h"
#include "RCP_Host/RCP_Host.h"

namespace LRI::RCI {
    using HostTime = std::chrono::system_clock::time_point;
    using TargetTime = uint32_t;
    constexpr auto& getHostTime = std::chrono::system_clock::now;


    struct CombinedTime {
        TargetTime ttime;
        HostTime htime;
        explicit CombinedTime(TargetTime ttime, const HostTime& htime = getHostTime()) : ttime(ttime), htime(htime) {}
    };

    using TargetFloat = std::tuple<const std::vector<CombinedTime>*, const std::vector<float>*>;
    using TargetUint = std::tuple<const std::vector<CombinedTime>*, const std::vector<uint8_t>*>;

    using HostString = std::tuple<const std::vector<HostTime>*, const std::vector<std::string>*>;
    using HostUint = std::tuple<const std::vector<HostTime>*, const std::vector<uint8_t>*>;
    using HostFloat = std::tuple<const std::vector<HostTime>*, const std::vector<float>*>;
    using PromptResponse =
        std::tuple<const std::vector<HostTime>*, const std::vector<float>*, const std::vector<uint8_t>*>;
    using TimePointList = std::vector<HostTime>;
    using StepperWritesList =
        std::tuple<const std::vector<HostTime>*, const std::vector<uint8_t>*, const std::vector<float>*>;
    using ReadRequestsList = std::vector<std::pair<HostTime, HardwareQualifier>>;
    using UintList = std::vector<uint8_t>;

    class EventLog {
        struct {
            // Timestamps are stored separately from the data channel data, since for up to 4 channels in one single
            // device, we would be storing the time information 4 separate times. Timepoints are associated to
            // datapoints based on array index, since all datapoints are added to along with the time point at the
            // same time
            std::map<HardwareQualifier, std::vector<CombinedTime>> timestamps;
            std::map<HardwareChannel, std::vector<float>> floats;
            std::map<HardwareChannel, std::vector<uint8_t>> uints;
        } target;

        struct {
            std::map<HardwareQualifier, std::vector<HostTime>> acttimestamps;
            std::map<HardwareQualifier, std::vector<std::string>> strings; // For prompts and logs
            std::map<HardwareQualifier, std::vector<uint8_t>> act_uints;
            std::map<HardwareQualifier, std::vector<float>> act_floats;

            std::map<HardwareChannel, std::vector<HostTime>> ctrltimestamps;
            std::map<HardwareChannel, std::vector<float>> tares;
            std::map<HardwareChannel, std::vector<uint8_t>> ctrl_uints;

            ReadRequestsList readReqs;
        } host;

        std::vector<uint8_t> rxbytes;
        std::vector<uint8_t> txbytes;

    public:
        EventLog();
        ~EventLog() = default;

        void createDevice(const HardwareQualifier& qual);
        void clear();

        // Received from target
        void addTestState(const RCP_TestData& td);
        void addPromptRequest(const RCP_PromptInputRequest& preq);
        void addTargetLog(const RCP_TargetLogData& log);
        void addByteData(const RCP_ByteData& data);
        void add1F(const RCP_1F& f1);
        void add2F(const RCP_2F& f2);
        void add3F(const RCP_3F& f3);
        void add4F(const RCP_4F& f4);

        // Sent from host
        void addTestStart(uint8_t testnum);
        void addTestStop();
        void addTestPauseUnpause();
        void addHeartbeatSet(uint8_t time);
        void addHeartbeat();
        void addDStreamChange(bool streaming);
        void addESTOP();
        void addHWRST();
        void addTMRST();

        void addPromptResponse(float val);
        void addPromptResponse(bool val);

        void addAActWrite(uint8_t id, float val);
        void addMotorWrite(uint8_t id, float val);
        void addStepperWrite(uint8_t id, RCP_StepperControlMode mode, float val);
        void addSActWrite(uint8_t id, RCP_SimpleActuatorState val);
        void addDActWrite(uint8_t id, uint8_t val);

        void addReadReq(const HardwareQualifier& qual);
        void addTare(const HardwareChannel& ch, float off);

        // Logs all raw RCP data sent and received for later inspection
        void addReceived(const void* data, size_t length);
        void addSent(const void* data, size_t length);

        // Getters
        [[nodiscard]] TargetUint getReportedTestStateChannel(TestStateChannels::Channels ch);

        [[nodiscard]] HostString getPromptRequests() const;
        [[nodiscard]] PromptResponse getPromptResponses() const;

        [[nodiscard]] HostString getLogs() const;

        [[nodiscard]] TargetUint getChannelUintData(const HardwareChannel& ch) const;
        [[nodiscard]] TargetFloat getChannelFloatData(const HardwareChannel& ch) const;

        [[nodiscard]] HostUint getRequestedRunningStates() const;
        [[nodiscard]] const UintList* getRequestedTestStartIDs() const;
        [[nodiscard]] HostUint getRequestedHeartbeatTimeSet() const;
        [[nodiscard]] const TimePointList* getHeartbeats() const;
        [[nodiscard]] HostUint getRequestedDStreams() const;
        [[nodiscard]] const TimePointList* getRequestedHWResets() const;
        [[nodiscard]] const TimePointList* getRequestedTimeResets() const;
        [[nodiscard]] HostFloat getRequestedAActWrites(uint8_t id) const;
        [[nodiscard]] HostFloat getRequestedMotorWrites(uint8_t id) const;
        [[nodiscard]] StepperWritesList getRequestedStepperWrites(uint8_t id) const;
        [[nodiscard]] HostUint getRequestedSActWrites(uint8_t id) const;
        [[nodiscard]] HostUint getRequestedDActWrites(uint8_t id) const;
        [[nodiscard]] HostFloat getRequestedTares(const HardwareChannel& ch) const;

        [[nodiscard]] const ReadRequestsList* getReadRequests() const;

        [[nodiscard]] const UintList* getRXBytes() const;
        [[nodiscard]] const UintList* getTXBytes() const;
    };
} // namespace LRI::RCI

#endif // LRI_CONTROL_PANEL_EVENTLOG_H
