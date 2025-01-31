#include "RCP_Host/RCP_Host.h"
#include "RCP_Host_Impl.h"

#include <string>
#include <UI/Steppers.h>

#include "UI/TargetChooser.h"
#include "UI/Solenoids.h"
#include "UI/TestControl.h"
#include "UI/SensorReadings.h"
#include "UI/CustomData.h"

namespace LRI::RCI {
    // This file contains all the callbacks needed for RCP. They simply forward data to the respective window
    RCP_LibInitData callbacks = {
            .sendData = sendData,
            .readData = readData,
            .processTestUpdate = processTestUpdate,
            .processSolenoidData = processSolenoidData,
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
        TestControl::getInstance()->receiveRCPUpdate(data);
        return 0;
    }

    int processSolenoidData(const RCP_SolenoidData data) {
        Solenoids::getInstance()->receiveRCPUpdate(data);
        return 0;
    }

    int processSerialData(const RCP_CustomData data) {
        CustomData::getInstance()->recevieRCPUpdate(data);
        return 0;
    }

    int processOneFloat(RCP_OneFloat data) {
        SensorReadings::getInstance()->receiveRCPUpdate(data);
        return 0;
    }

    int processTwoFloat(RCP_TwoFloat data) {
        if(data.devclass == RCP_DEVCLASS_STEPPER) {
            Steppers::getInstance()->receiveRCPUpdate(data);
            return 0;
        }

        SensorReadings::getInstance()->receiveRCPUpdate(data);
        return 0;
    }

    int processThreeFloat(RCP_ThreeFloat data) {
        SensorReadings::getInstance()->receiveRCPUpdate(data);
        return 0;
    }

    int processFourFloat(RCP_FourFloat data) {
        SensorReadings::getInstance()->receiveRCPUpdate(data);
        return 0;
    }
}
