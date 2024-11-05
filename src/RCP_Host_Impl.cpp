#include "RCP_Host/RCP_Host.h"
#include "RCP_Host_Impl.h"

#include "UI/COMPortChooser.h"

namespace LRI::RCI {
    RCP_LibInitData callbacks = {
        .sendData = sendData,
        .readData = readData,
        .processTestUpdate = processTestUpdate,
        .processSolenoidData = processSolenoidData,
        .processStepperData = processStepperData,
        .processTransducerData = processTransducerData,
        .processGPSData = processGPSData,
        .processMagnetometerData = processMagnetometerData,
        .processAMPressureData = processAMPressureData,
        .processAMTemperatureData = processAMTemperatureData,
        .processAccelerationData = processAccelerationData,
        .processGyroData = processGyroData,
        .processSerialData = processSerialData
    };
    size_t sendData(const void* data, size_t length) {
        return COMPortChooser::getInstance()->getComPort()->sendData(data, length);
    }

    size_t readData(void* data, size_t bufferSize) {
        return COMPortChooser::getInstance()->getComPort()->readData(data, bufferSize);
    }

    int processTestUpdate(const RCP_TestData data) {
        return 0;
    }

    int processSolenoidData(const RCP_SolenoidData data) {
        return 0;
    }

    int processStepperData(const RCP_StepperData data) {
        return 0;
    }

    int processTransducerData(const RCP_TransducerData data) {
        return 0;
    }

    int processGPSData(const RCP_GPSData data) {
        return 0;
    }

    int processMagnetometerData(const RCP_AxisData data) {
        return 0;
    }

    int processAMPressureData(const RCP_AMPressureData data) {
        return 0;
    }

    int processAMTemperatureData(const RCP_AMTemperatureData data) {
        return 0;
    }

    int processAccelerationData(const RCP_AxisData data) {
        return 0;
    }

    int processGyroData(const RCP_AxisData data) {
        return 0;
    }

    int processSerialData(const RCP_SerialData data) {
        return 0;
    }
}
