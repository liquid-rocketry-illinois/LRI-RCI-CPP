#ifndef RCP_HOST_IMPL_H
#define RCP_HOST_IMPL_H

#include "RCP_Host/RCP_Host.h"

namespace LRI::RCI {
    // All callbacks for RCP
    extern RCP_LibInitData callbacks;
    size_t sendData(const void* data, size_t length);
    size_t readData(void* data, size_t bufferSize);
    int processTestUpdate(RCP_TestData data);
    int processBoolData(RCP_BoolData data);
    int processSimpleActuatorData(RCP_SimpleActuatorData data);
    int processPromptInput(RCP_PromptInputRequest request);
    int processSerialData(RCP_CustomData data);
    int processOneFloat(RCP_OneFloat data);
    int processTwoFloat(RCP_TwoFloat data);
    int processThreeFloat(RCP_ThreeFloat data);
    int processFourFloat(RCP_FourFloat data);
}

#endif //RCP_HOST_IMPL_H
