#ifndef TESTSTATE_H
#define TESTSTATE_H

#include <atomic>
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
    bool startTest(uint8_t number);
    bool stopTest();
    bool pause();
    bool setHeartbeatTime(uint8_t time);
    bool setDataStreaming(bool stream);
    void setResetTimeOnTestStart(bool reset);
    bool deviceReset();

    // To be called from main. Handles heartbeats
    void update();

    // Clear inited
    void reset();

    // Receive updates from rcp
    int receiveRCPUpdate(RCP_TestData testState);
} // namespace LRI::RCI

#endif // TESTSTATE_H
