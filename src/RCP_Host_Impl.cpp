#include "RCP_Host/RCP_Host.h"
#include "RCP_Host_Impl.h"

#include <UI/StepperViewer.h>

#include "UI/TargetChooser.h"
#include "UI/SolenoidViewer.h"
#include "UI/TestStateViewer.h"
#include "UI/SensorViewer.h"
#include "UI/RawViewer.h"
#include "UI/PromptViewer.h"

namespace LRI::RCI {
    // This file contains all the callbacks needed for RCP. They simply forward data to the respective window
    RCP_LibInitData callbacks = {
        .sendData = sendData,
        .readData = readData,
        .processTestUpdate = processTestUpdate,
        .processSolenoidData = processSolenoidData,
        .processPromptInput = processPromptInput,
        .processSerialData = processSerialData,
        .processOneFloat = processOneFloat,
        .processTwoFloat = processTwoFloat,
        .processThreeFloat = processThreeFloat,
        .processFourFloat = processFourFloat,
    };

    size_t sendData(const void* data, size_t length) {
        return TargetChooser::getInstance()->getInterface()->sendData(data, length);
    }

    size_t readData(void* data, size_t bufferSize) {
        return TargetChooser::getInstance()->getInterface()->readData(data, bufferSize);
    }

    int processTestUpdate(const RCP_TestData data) {
        TestStateViewer::getInstance()->receiveRCPUpdate(data);
        return 0;
    }

    int processSolenoidData(const RCP_SolenoidData data) {
        SolenoidViewer::getInstance()->receiveRCPUpdate(data);
        return 0;
    }

    int processPromptInput(RCP_PromptInputRequest request) {
        PromptViewer::getInstance()->setPrompt(request);
        return 0;
    }

    int processSerialData(const RCP_CustomData data) {
        RawViewer::getInstance()->recevieRCPUpdate(data);
        return 0;
    }

    int processOneFloat(RCP_OneFloat data) {
        SensorViewer::getInstance()->receiveRCPUpdate(data);
        return 0;
    }

    int processTwoFloat(RCP_TwoFloat data) {
        if(data.devclass == RCP_DEVCLASS_STEPPER) StepperViewer::getInstance()->receiveRCPUpdate(data);
        else SensorViewer::getInstance()->receiveRCPUpdate(data);
        return 0;
    }

    int processThreeFloat(RCP_ThreeFloat data) {
        SensorViewer::getInstance()->receiveRCPUpdate(data);
        return 0;
    }

    int processFourFloat(RCP_FourFloat data) {
        SensorViewer::getInstance()->receiveRCPUpdate(data);
        return 0;
    }
}
