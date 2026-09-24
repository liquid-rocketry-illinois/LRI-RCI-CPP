#include "hardware/hwctrl.h"

#include <chrono>
#include <set>
#include <vector>

#include "hardware/HardwareQualifier.h"
#include "utils.h"

#define CHECKV(val)                                                                                                    \
    do {                                                                                                               \
        if(val == RCP_ERR_IO_SEND) elog->addError(Error::LERROR, "RCP encountered IO send error in {}", __func__);     \
    }                                                                                                                  \
    while(0)
#define CHECK(x)                                                                                                       \
    do {                                                                                                               \
        RCP_Error err = x;                                                                                             \
        CHECKV(x);                                                                                                     \
    }                                                                                                                  \
    while(0)

namespace {
    using namespace LRI::RCI;

    std::chrono::system_clock::time_point lastHeartbeat;
    // Heartbeat time, in millis
    uint8_t originalHeartbeat;
    unsigned heartbeatTime;
    float heartbeatThreashold = 0.9f;
    bool isDStream = false;

    RCP_Context rctx;
    RCP_Interface* interf = nullptr;
    EventLog* elog = nullptr;

    std::set<HardwareQualifier> quals;
    std::vector<TargetTest> tests;

    bool targetReady = false;
} // namespace

// RCP callbacks
namespace {
    using namespace LRI::RCI;

    size_t readData(void* data, size_t length) {
        size_t read = interf->readData(data, length);
        elog->addReceived(data, length);
        return read;
    }

    size_t sendData(const void* data, size_t length) {
        size_t sent = interf->sendData(data, length);
        elog->addSent(data, length);
        return sent;
    }

    RCP_Error testData(RCP_TestData data) {
        elog->addTestState(data);
        if(data.isInited) targetReady = true;
        return RCP_ERR_SUCCESS;
    }

    RCP_Error byteData(RCP_ByteData data) {
        elog->addByteData(data);
        return RCP_ERR_SUCCESS;
    }

    RCP_Error promptInput(RCP_PromptInputRequest pirq) {
        elog->addPromptRequest(pirq);
        return RCP_ERR_SUCCESS;
    }

    RCP_Error targetLog(RCP_TargetLogData data) {
        elog->addTargetLog(data);
        return RCP_ERR_SUCCESS;
    }

    RCP_Error F1(RCP_1F f1) {
        elog->add1F(f1);
        return RCP_ERR_SUCCESS;
    }

    RCP_Error F2(RCP_2F f2) {
        elog->add2F(f2);
        return RCP_ERR_SUCCESS;
    }

    RCP_Error F3(RCP_3F f3) {
        elog->add3F(f3);
        return RCP_ERR_SUCCESS;
    }

    RCP_Error F4(RCP_4F f4) {
        elog->add4F(f4);
        return RCP_ERR_SUCCESS;
    }

    RCP_LibInitData rcpInit = {.sendData = sendData,
                               .readData = readData,
                               .processTestUpdate = testData,
                               .processByteData = byteData,
                               .processPromptInput = promptInput,
                               .processTargetLog = targetLog,
                               .processOneFloat = F1,
                               .processTwoFloat = F2,
                               .processThreeFloat = F3,
                               .processFourFloat = F4};
} // namespace

namespace LRI::RCI::hwctrl {
    int POLLS_PER_UPDATE = 25;

    void start(RCP_Interface* interf, const TargetConfig& config) {
        ::interf = interf;

        lastHeartbeat = std::chrono::system_clock::now();
        heartbeatTime = 0;
        targetReady = false;
        rctx = RCP_createContext(rcpInit);
        RCP_setContext(rctx);
        RCP_setChannel(RCP_CH_ZERO);

        elog = new EventLog();

        for(const auto& devset : config.devices) {
            RCP_DeviceClass devclass = devset.devclass;

            for(size_t i = 0; i < devset.ids.size(); i++) {
                HardwareQualifier qual = {devclass, devset.ids[i], devset.names[i]};
                if(quals.contains(qual)) {
                    elog->addError(Error::LWARNING, "Repeated hardware qualifier: {}", qual);
                    continue;
                }

                quals.emplace(qual);
            }
        }

        for(const auto& qual : quals) elog->createDevice(qual);

        tests = config.tests;
    }

    void update() {
        if(!interf) return;

        if(heartbeatTime != 0) {
            auto now = std::chrono::system_clock::now();
            if(now - lastHeartbeat > std::chrono::milliseconds(heartbeatTime)) {
                CHECK(RCP_sendHeartbeat());
                elog->addHeartbeat();
                lastHeartbeat = now;
            }
        }

        for(int i = 0; i < POLLS_PER_UPDATE; i++) {
            if(!interf->pktAvailable()) break;
            RCP_Error err = RCP_poll();

            switch(err) {
            case RCP_ERR_IO_RCV:
                elog->addError(Error::LERROR, "RCP encountered IO receive error");
                break;

            case RCP_ERR_AMALG_SUBUNIT:
                elog->addError(Error::LWARNING, "RCP encountered non-amalgable subunit inside AMALGAMATE IU");
                break;

            case RCP_ERR_AMALG_NESTING:
                elog->addError(Error::LWARNING, "RCP encountered a nested AMALGAMATE IU");
                break;

            case RCP_ERR_INVALID_DEVCLASS:
                elog->addError(Error::LWARNING, "RCP encountered unknown device class");
                break;

            default:
            }
        }
    }

    void end() {
        RCP_destroyContext(rctx);
        rctx = nullptr;

        delete elog;
        elog = nullptr;

        delete interf;
        interf = nullptr;

        quals.clear();
        tests.clear();
    }

    size_t interfBytesWaiting() {
        if(interf == nullptr) return 0;
        return interf->bytesWaiting();
    }

    bool isOpen() { return interf != nullptr; }
    bool isTargetReady() { return targetReady; }
    const EventLog* getELog() { return elog; }
    void addError(Error&& e) { elog->addError(std::move(e)); }
    const std::set<HardwareQualifier>& getQuals() { return quals; }
    const std::vector<TargetTest>& getTests() { return tests; }
    RCP_TestRunningState getTestState() {
        auto vals = elog->getTestInformation(TestStateChannels::T_TEST_RUN_STATE).values;
        if(vals->empty()) return RCP_TEST_STOPPED;
        return static_cast<RCP_TestRunningState>(vals->back());
    }
    bool isDataStreaming() { return isDStream; }
    uint8_t getHeartbeatTime() { return originalHeartbeat; }
    float getHeartbeatThreashold() { return heartbeatThreashold; }

    // float heartbeatThreashold() { return heart; }

    void refresh(const HardwareQualifier& qual) {
        elog->addReadReq(qual);
        RCP_Error err = RCP_requestGeneralRead(qual.devclass, qual.id);
        CHECKV(err);
        if(err == RCP_ERR_INVALID_DEVCLASS)
            elog->addError(Error::LWARNING, "Refresh requested for nonrefreshable device {}", qual);
    }

    void requestTare(const HardwareChannel& qual, float value) {
        elog->addTare(qual, value);
        RCP_Error err = RCP_requestTareConfiguration(qual.devclass, qual.id, qual.channel, value);
        CHECKV(err);
        if(err == RCP_ERR_INVALID_DEVCLASS)
            elog->addError(Error::LWARNING, "Tare requested for non-tare-able device {}", qual);
    }

    void startTest(uint8_t number, bool resetTime) {
        for(const auto& test : tests) {
            if(test.id == number) {
                elog->addTestStart(number);
                if(resetTime) CHECK(RCP_deviceTimeReset());
                CHECK(RCP_startTest(number));
                return;
            }
        }

        elog->addError(Error::LWARNING, "Test start requested for nonexistent test with id {}", number);
    }

    void stopTest() {
        elog->addTestStop();
        CHECK(RCP_stopTest());
    }

    void pauseTest() {
        elog->addTestPauseUnpause();
        CHECK(RCP_pauseUnpauseTest());
    }

    void setHeartbeatTime(uint8_t time) {
        elog->addHeartbeatSet(time);
        originalHeartbeat = time;
        setHeartbeatThreshold(heartbeatThreashold);
        CHECK(RCP_setHeartbeatTime(time));
    }

    void setHeartbeatThreshold(float threashold) {
        heartbeatThreashold = threashold;
        if(originalHeartbeat == 0) heartbeatTime = 0;
        else {
            heartbeatTime =
                static_cast<unsigned>(static_cast<float>(originalHeartbeat) * 1000.0f * heartbeatThreashold);
        }
    }

    void setDataStreaming(bool stream) {
        isDStream = stream;
        elog->addDStreamChange(stream);
        CHECK(RCP_setDataStreaming(stream));
    }

    void deviceReset() {
        elog->addHWRST();
        CHECK(RCP_deviceReset());
    }

    void timeReset() {
        elog->addTMRST();
        CHECK(RCP_deviceTimeReset());
    }

    void ESTOP() {
        elog->addESTOP();
        CHECK(RCP_sendEStop());
    }

    void writeDA(uint8_t id, uint8_t value) {
        elog->addDActWrite(id, value);
        CHECK(RCP_sendDiscreteActuatorWrite(id, value));
    }

    void writeStepper(uint8_t id, RCP_StepperControlMode mode, float value) {
        elog->addStepperWrite(id, mode, value);
        CHECK(RCP_sendStepperWrite(id, mode, value));
    }

    const std::string& promptString() {
        static const std::string EMPTY = "NONE";
        if(elog == nullptr || elog->getPromptRequests().values->empty()) return EMPTY;
        return elog->getPromptRequests().values->back();
    }

    RCP_PromptDataType promptType() { return RCP_getActivePromptType(); }

    void promptRespond(float value) {
        elog->addPromptResponse(value);
        RCP_Error err = RCP_promptRespondFloat(value);
        CHECKV(err);
        if(err == RCP_ERR_NO_ACTIVE_PROMPT)
            elog->addError(Error::LWARNING, "Prompt response sent without any active prompt");
    }

    void promptRespond(bool value) {
        elog->addPromptResponse(value);
        RCP_Error err = RCP_promptRespondGONOGO(value ? RCP_GONOGO_GO : RCP_GONOGO_NOGO);
        CHECKV(err);
        if(err == RCP_ERR_NO_ACTIVE_PROMPT)
            elog->addError(Error::LWARNING, "Prompt response sent without any active prompt");
    }

    void writeAA(uint8_t id, float degrees) {
        elog->addAActWrite(id, degrees);
        CHECK(RCP_sendAngledActuatorWrite(id, degrees));
    }

    void writeMotor(uint8_t id, float value) {
        elog->addMotorWrite(id, value);
        CHECK(RCP_sendMotorWrite(id, value));
    }
} // namespace LRI::RCI::hwctrl
