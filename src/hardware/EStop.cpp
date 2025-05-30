#include "hardware/EStop.h"
#include "RCP_Host/RCP_Host.h"
#include "hardware/TestState.h"

namespace LRI::RCI {
    EStop* EStop::getInstance() {
        static EStop* instance = nullptr;
        if(instance == nullptr) instance = new EStop();
        return instance;
    }

    void EStop::ESTOP() {
        if(!TestState::getInited()) return;
        RCP_sendEStop();
        isStopped = true;
    }

    bool EStop::isEstopped() const { return isStopped; }

    void EStop::receiveRCPUpdate(bool _isEStopped) { isStopped = _isEStopped; }
} // namespace LRI::RCI
