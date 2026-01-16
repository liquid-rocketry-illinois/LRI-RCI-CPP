#ifndef TESTSTATE_H
#define TESTSTATE_H

#include <map>

#include "RCP_Host/RCP_Host.h"
#include "utils.h"

// Singleton for containing all test state information
namespace LRI::RCI::TestState {
    [[nodiscard]] bool getInited();

    void setTests(const std::map<uint8_t, std::string>& testlist);
    [[nodiscard]] const std::map<uint8_t, std::string>* getTestOptions();

    // Getters for class members
    [[nodiscard]] uint8_t getActiveTest();
    [[nodiscard]] RCP_TestRunningState getState();
    [[nodiscard]] uint8_t getHeartbeatTime();
    [[nodiscard]] bool getDataStreaming();

    // Methods to send requests to target
    void startTest(uint8_t number);
    void stopTest();
    void pause();
    void setHeartbeatTime(uint8_t time);
    void setDataStreaming(bool stream);
    void setResetTimeOnTestStart(bool reset);
    void deviceReset();
    void ESTOP();

    // To be called from main. Handles heartbeats
    void update();

    // Clear inited
    void reset();

    // Receive updates from rcp
    RCP_Error receiveRCPUpdate(RCP_TestData testState);
} // namespace LRI::RCI

#endif // TESTSTATE_H
