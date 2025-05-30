#ifndef TESTSTATE_H
#define TESTSTATE_H

#include <atomic>
#include <map>

#include "RCP_Host/RCP_Host.h"
#include "utils.h"

// Singleton for containing all test state information
namespace LRI::RCI {
    class TestState {
        // A global variable for indicating if the target has sent an init packet. This is an atomic so that
        // the interfaces can determine if they can start sending data from the buffers
        static std::atomic_bool inited;

        // Current test state storage
        RCP_TestRunningState state;
        std::map<uint8_t, std::string> tests;
        uint8_t activeTest;
        uint8_t heartbeatTime;
        bool dataStreaming;
        bool resetTimeOnStart;

        // Timer for heartbeats
        StopWatch heartbeat;

        TestState() = default;
        ~TestState() = default;

    public:
        // Get singleton instance and inited state
        static TestState* getInstance();
        [[nodiscard]] static bool getInited();

        void setTests(const std::map<uint8_t, std::string>& testlist);
        [[nodiscard]] const std::map<uint8_t, std::string>* getTestOptions() const;

        // Getters for class members
        [[nodiscard]] uint8_t getActiveTest() const;
        [[nodiscard]] RCP_TestRunningState getState() const;
        [[nodiscard]] uint8_t getHeartbeatTime() const;
        [[nodiscard]] bool getDataStreaming() const;

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

        // Receive updates from rcp
        void receiveRCPUpdate(const RCP_TestData& testState);
    };
} // namespace LRI::RCI

#endif // TESTSTATE_H
