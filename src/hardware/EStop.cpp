#include "hardware/EStop.h"
#include "RCP_Host/RCP_Host.h"
#include "hardware/TestState.h"

namespace LRI::RCI::EStop {
    static bool isStopped;

    void ESTOP() {
        if(!TestState::getInited()) return;
        RCP_sendEStop();
        isStopped = true;
    }

    bool isEstopped() { return isStopped; }

    void receiveRCPUpdate(bool _isEStopped) { isStopped = _isEStopped; }
} // namespace LRI::RCI::EStop
