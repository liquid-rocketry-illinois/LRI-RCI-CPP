#ifndef TESTSTATEVIEWER_H
#define TESTSTATEVIEWER_H

#include "../WModule.h"

namespace LRI::RCI {
    // A window module for controlling the test state of the target
    class TestStateViewer : public WModule {
        // The in-modification values for test state
        int inputHeartbeatRate;
        uint8_t activeTest;
        bool dstream;
        bool doHeartbeats;
        bool resetTimeOnTestStart;

        // Timer for start button
        StopWatch startTimer;
        StopWatch dResetTimer;

    public:
        TestStateViewer();
        ~TestStateViewer() override = default;

        // Overridden render function
        void render() override;
    };
} // namespace LRI::RCI

#endif // TESTSTATEVIEWER_H
