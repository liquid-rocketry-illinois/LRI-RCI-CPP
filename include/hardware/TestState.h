#ifndef TESTSTATE_H
#define TESTSTATE_H

#include <atomic>

#include "RCP_Host/RCP_Host.h"

#include "utils.h"

// Singleton for containing all test state information
namespace LRI::RCI {
    class TestState {
        // A global variable for indicating if the target has sent an init packet
        static std::atomic_bool inited;

        // Current test state storage
        RCP_TestRunningState state;
        uint8_t testNum;
        uint8_t heartbeatTime;
        bool dataStreaming;

        // Timer for heartbeats
        StopWatch heartbeat;

        TestState() = default;
        ~TestState() = default;

    public:
        // Get singleton instance and inited state
        static TestState* getInstance();
        [[nodiscard]] static bool getInited();

        // Getters for class members
        [[nodiscard]] int getTestNum() const;
        [[nodiscard]] RCP_TestRunningState getState() const;
        [[nodiscard]] uint8_t getHeartbeatTime() const;
        [[nodiscard]] bool getDataStreaming() const;

        // Methods to send requests to target
        bool startTest(uint8_t number);
        bool stopTest();
        bool pause();
        bool setHeartbeatTime(uint8_t time);
        bool setDataStreaming(bool stream);
        bool deviceReset();

        // To be called from main. Handles heartbeats
        void update();

        // Receive updates from rcp
        void receiveRCPUpdate(const RCP_TestData& testState);
    };
}

#endif //TESTSTATE_H
