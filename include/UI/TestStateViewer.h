#ifndef TESTCONTROL_H
#define TESTCONTROL_H

#include <utils.h>
#include <RCP_Host/RCP_Host.h>

#include "BaseUI.h"

namespace LRI::RCI {
    // A window for controlling the test state of the target
    class TestStateViewer : public BaseUI {
        // The modified hearbeat rate
        int inputHeartbeatRate{};
        int inputTestNum{};
        bool dstream{};
        bool doHeartbeats{};

    public:
        TestStateViewer() = default;
        ~TestStateViewer() override = default;

        // Overridden render function
        void render() override;

        // Custom reset
        void reset() override;
    };
}

#endif //TESTCONTROL_H
