#ifndef LRI_CONTROL_PANEL_HWCTRL_H
#define LRI_CONTROL_PANEL_HWCTRL_H

#include <set>
#include <vector>

#include "interfaces/RCP_Interface.h"

#include "hardware/EventLog.h"
#include "hardware/json.h"

namespace LRI::RCI::hwctrl {
    extern int POLLS_PER_UPDATE;

    void start(RCP_Interface* interf, const TargetConfig& config);
    void update();
    void end();

    size_t interfBytesWaiting();
    bool isOpen();
    bool isTargetReady();
    const EventLog* getELog();

    void addError(Error&& e);
    template<typename... Args>
    void addError(Error::Level level, std::format_string<Args...> fmt, Args&&... args) {
        addError(Error(level, fmt, std::forward<Args>(args)...));
    }

    const std::set<HardwareQualifier>& getQuals();
    const std::vector<TargetTest>& getTests();

    // Some helper getters
    RCP_TestRunningState getTestState();
    bool isDataStreaming();
    uint8_t getHeartbeatTime();
    float getHeartbeatThreashold();

    void refresh(const HardwareQualifier& qual);
    void requestTare(const HardwareChannel& qual, float value);

    void startTest(uint8_t number, bool resetTime = false);
    void stopTest();
    void pauseTest();
    void setHeartbeatTime(uint8_t time);
    void setHeartbeatThreshold(float threashold);
    void setDataStreaming(bool stream);
    void deviceReset();
    void timeReset();
    void ESTOP();

    void writeDA(uint8_t id, uint8_t value);
    void writeStepper(uint8_t id, RCP_StepperControlMode mode, float value);

    const std::string& promptString();
    RCP_PromptDataType promptType();
    void promptRespond(float value);
    void promptRespond(bool value);

    void writeAA(uint8_t id, float degrees);
    void writeMotor(uint8_t id, float value);
} // namespace LRI::RCI::hwctrl

#endif // LRI_CONTROL_PANEL_HWCTRL_H
