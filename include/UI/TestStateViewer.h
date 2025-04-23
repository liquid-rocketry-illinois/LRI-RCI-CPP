#ifndef TESTSTATEVIEWER_H
#define TESTSTATEVIEWER_H

#include "WModule.h"

namespace LRI::RCI {
    // A window module for controlling the test state of the target
    class TestStateViewer : public WModule {
        static int CLASSID;
        const int classid;

        // The in-modification values for test state
        int inputHeartbeatRate;
        uint8_t activeTest;
        bool dstream;
        bool doHeartbeats;

        // Timer for start button
        StopWatch startTimer;

    public:
        TestStateViewer();
        ~TestStateViewer() override = default;

        // Overridden render function
        void render() override;
    };
}

#endif //TESTSTATEVIEWER_H
