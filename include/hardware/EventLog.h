#ifndef LRI_CONTROL_PANEL_EVENTLOG_H
#define LRI_CONTROL_PANEL_EVENTLOG_H

#include <chrono>
#include <map>
#include <string>
#include <vector>

#include "HardwareQualifier.h"
#include "RCP_Host/RCP_Host.h"

namespace LRI::RCI {
    template<typename T>
    struct DataPoint {
        std::chrono::system_clock::time_point systime;
        uint32_t targetMillis = 0;
        T data;

        explicit DataPoint(const T& data) : DataPoint(0, data) {}

        DataPoint(uint32_t targetMillis, const T& data) :
            DataPoint(std::chrono::system_clock::now(), targetMillis, data) {}

        DataPoint(const std::chrono::system_clock::time_point& systime, uint32_t targetMillis, const T& data) :
            systime(systime), targetMillis(targetMillis), data(data) {}
    };

    struct TimePoint {
        std::chrono::system_clock::time_point systime;
        uint32_t targetMillis = 0;

        TimePoint(const std::chrono::system_clock::time_point& systime, uint32_t targetMillis) :
            systime(systime), targetMillis(targetMillis) {}

        explicit TimePoint(uint32_t targetMillis) : TimePoint(std::chrono::system_clock::now(), targetMillis) {}
    };

    struct PromptRequest {
        RCP_PromptDataType type;
        std::string prompt;
    };

    struct PromptResponse {
        size_t index;
        union {
            bool bdata;
            float fdata;
        };
    };

    struct StepperWrite {
        RCP_StepperControlMode mode;
        float value;
    };

    class EventLog {
        enum class MiscEvents { TEST_START, TEST_PROG, HEARTBEAT_SET, HEARTBEAT, INITED, DSTREAM, HWRST, TMRST };

        struct {
            std::vector<DataPoint<RCP_TestRunningState>> testRunningState;
            std::map<MiscEvents, std::vector<DataPoint<uint8_t>>> misc;
            std::vector<DataPoint<std::string>> logs;
            std::vector<DataPoint<PromptRequest>> prompts;

            // Timestamps are stored separately from the data channel data, since for up to 4 channels in one single
            // device, we would be storing the time information 4 separate times. Timepoints are associated to
            // datapoints based on array index, since all datapoints are added to along with the time point at the
            // same time
            std::map<HardwareQualifier, std::vector<TimePoint>> timestamps;
            std::map<HardwareChannel, std::vector<float>> floats;
            std::map<HardwareChannel, std::vector<bool>> bools;
        } target;

        struct {
            std::vector<DataPoint<RCP_TestRunningState>> testRunningState;
            std::map<MiscEvents, std::vector<DataPoint<uint8_t>>> misc;
            std::vector<DataPoint<PromptResponse>> promptResponses;
            std::vector<DataPoint<HardwareQualifier>> readReqs;

            std::map<uint8_t, std::vector<DataPoint<float>>> aActWrites;
            std::map<uint8_t, std::vector<DataPoint<StepperWrite>>> stepperWrites;
            std::map<uint8_t, std::vector<DataPoint<bool>>> sActWrites;
        } host;

        std::vector<uint8_t> receivedBytes;
        std::vector<uint8_t> sentBytes;

    public:
        [[nodiscard]] static EventLog& getGlobalLog();

        void createDevice(const HardwareQualifier& qual);
        void clear();

        void addTestState(const RCP_TestData& td);
        void addSimpleActuator(const RCP_SimpleActuatorData& sact);
        void addBoolData(const RCP_BoolData& bdata);
        void addTargetLog(const RCP_TargetLogData& log);
        void addPromptRequest(const RCP_PromptInputRequest& preq);
        void add1F(const RCP_1F& f1);
        void add2F(const RCP_2F& f2);
        void add3F(const RCP_3F& f3);
        void add4F(const RCP_4F& f4);

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
        void addStepperWrite(uint8_t id, RCP_StepperControlMode mode, float val);
        void addSActWrite(uint8_t id, RCP_SimpleActuatorState val);
        void addReadReq(const HardwareQualifier& qual);

        void addReceived(const void* data, size_t length);
        void addSent(const void* data, size_t length);

        [[nodiscard]] const auto& getReportedTestStates() const { return target.testRunningState; }
        [[nodiscard]] const auto& getReportedActiveTest() const { return target.misc.at(MiscEvents::TEST_START); }
        [[nodiscard]] const auto& getReportedTestProgress() const { return target.misc.at(MiscEvents::TEST_PROG); }
        [[nodiscard]] const auto& getReportedHeartbeatTimes() const {
            return target.misc.at(MiscEvents::HEARTBEAT_SET);
        }
        [[nodiscard]] const auto& getReportedInited() const { return target.misc.at(MiscEvents::INITED); }
        [[nodiscard]] const auto& getReportedDStreaming() const { return target.misc.at(MiscEvents::DSTREAM); }
        [[nodiscard]] const auto& getLogs() const { return target.logs; }
        [[nodiscard]] const auto& getPrompts() const { return target.prompts; }
        [[nodiscard]] const auto& getSensorTimestamps() const { return target.timestamps; }
        [[nodiscard]] const auto& getFloats() const { return target.floats; }
        [[nodiscard]] const auto& getBools() const { return target.bools; }

        [[nodiscard]] const auto& getRequestedTestStates() const { return host.testRunningState; }
        [[nodiscard]] const auto& getRequestedActiveTest() const { return host.misc.at(MiscEvents::TEST_START); }
        [[nodiscard]] const auto& getRequestedHeartbeatTime() const { return host.misc.at(MiscEvents::HEARTBEAT_SET); }
        [[nodiscard]] const auto& getHeartbeats() const { return host.misc.at(MiscEvents::HEARTBEAT); }
        [[nodiscard]] const auto& getRequestedDStream() const { return host.misc.at(MiscEvents::DSTREAM); }
        [[nodiscard]] const auto& getHWRSTs() const { return host.misc.at(MiscEvents::HWRST); }
        [[nodiscard]] const auto& getTMRSTs() const { return host.misc.at(MiscEvents::TMRST); }
        [[nodiscard]] const auto& getPromptResponses() const { return host.promptResponses; }
        [[nodiscard]] const auto& getReadRequests() const { return host.readReqs; }
        [[nodiscard]] const auto& getAActWrites() const { return host.aActWrites; }
        [[nodiscard]] const auto& getStepperWrites() const { return host.stepperWrites; }
        [[nodiscard]] const auto& getSActWrites() const { return host.sActWrites; }

        [[nodiscard]] const auto& getReceived() const { return receivedBytes; }
        [[nodiscard]] const auto& getSent() const { return sentBytes; }
    };

    using FloatData = std::tuple<const std::vector<TimePoint>*, const std::vector<float>*>;
    using BoolData = std::tuple<const std::vector<TimePoint>*, const std::vector<bool>*>;

} // namespace LRI::RCI

#endif // LRI_CONTROL_PANEL_EVENTLOG_H
