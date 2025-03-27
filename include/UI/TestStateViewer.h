#ifndef TESTSTATEVIEWER_H
#define TESTSTATEVIEWER_H

#include <utils.h>
#include <RCP_Host/RCP_Host.h>

#include "BaseUI.h"

namespace LRI::RCI {
    // A window for controlling the test state of the target
    class TestStateViewer : public BaseUI {
        static int CLASSID;

        const int classid;
        // The modified hearbeat rate
        int inputHeartbeatRate;
        int inputTestNum;
        bool dstream;
        bool doHeartbeats;

    public:
        TestStateViewer();
        ~TestStateViewer() override = default;

        // Overridden render function
        void render() override;

        // Custom reset
        void reset() override;
    };
}

#endif //TESTSTATEVIEWER_H
