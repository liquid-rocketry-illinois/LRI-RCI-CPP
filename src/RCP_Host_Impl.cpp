#include "RCP_Host_Impl.h"
#include "RCP_Host/RCP_Host.h"

#include "hardware/BoolSensor.h"
#include "hardware/Prompt.h"
#include "hardware/RawData.h"
#include "hardware/Sensors.h"
#include "hardware/SimpleActuators.h"
#include "hardware/Steppers.h"
#include "hardware/TestState.h"

#include "UI/Windowlet.h"

namespace LRI::RCI {
    // This file contains all the callbacks needed for RCP. They simply forward data to the respective singletons
    RCP_LibInitData callbacks = {
        .sendData = sendData,
        .readData = readData,
        .processTestUpdate = processTestUpdate,
        .processBoolData = processBoolData,
        .processSimpleActuatorData = processSimpleActuatorData,
        .processPromptInput = processPromptInput,
        .processSerialData = processSerialData,
        .processOneFloat = processOneFloat,
        .processTwoFloat = processTwoFloat,
        .processThreeFloat = processThreeFloat,
        .processFourFloat = processFourFloat,
    };

    size_t sendData(const void* data, size_t length) {
        return ControlWindowlet::getInstance()->getInterf()->sendData(data, length);
    }

    size_t readData(void* data, size_t bufferSize) {
        return ControlWindowlet::getInstance()->getInterf()->readData(data, bufferSize);
    }

    int processTestUpdate(const RCP_TestData data) {
        TestState::getInstance()->receiveRCPUpdate(data);
        return 0;
    }

    int processBoolData(RCP_BoolData data) {
        return BoolSensors::getInstance()->receiveRCPUpdate({RCP_DEVCLASS_BOOL_SENSOR, data.ID, ""}, data.data);
    }


    int processSimpleActuatorData(const RCP_SimpleActuatorData data) {
        return SimpleActuators::getInstance()->receiveRCPUpdate({RCP_DEVCLASS_SIMPLE_ACTUATOR, data.ID, ""},
                                                                data.state == RCP_SIMPLE_ACTUATOR_ON);
    }

    int processPromptInput(RCP_PromptInputRequest request) {
        Prompt::getInstance()->receiveRCPUpdate(request);
        return 0;
    }

    int processSerialData(const RCP_CustomData data) {
        RawData::getInstance()->receiveRCPUpdate(data);
        return 0;
    }

    int processOneFloat(RCP_OneFloat data) {
        return Sensors::getInstance()->receiveRCPUpdate(data);
    }

    int processTwoFloat(RCP_TwoFloat data) {
        if(data.devclass == RCP_DEVCLASS_STEPPER)
            return Steppers::getInstance()->receiveRCPUpdate({RCP_DEVCLASS_STEPPER, data.ID, ""}, data.data[0], data.data[1]);
        return Sensors::getInstance()->receiveRCPUpdate(data);
    }

    int processThreeFloat(RCP_ThreeFloat data) {
        return Sensors::getInstance()->receiveRCPUpdate(data);
    }

    int processFourFloat(RCP_FourFloat data) {
        return Sensors::getInstance()->receiveRCPUpdate(data);
    }
} // namespace LRI::RCI
