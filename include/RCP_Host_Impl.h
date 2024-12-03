#ifndef RCP_HOST_IMPL_H
#define RCP_HOST_IMPL_H

#include "RCP_Host/RCP_Host.h"

namespace LRI::RCI {
    extern RCP_LibInitData callbacks;
    size_t sendData(const void* data, size_t length);
    size_t readData(void* data, size_t bufferSize);
    int processTestUpdate(const RCP_TestData data);
    int processSolenoidData(const RCP_SolenoidData data);
    int processStepperData(const RCP_StepperData data);
    int processTransducerData(const RCP_TransducerData data);
    int processGPSData(const RCP_GPSData data);
    int processMagnetometerData(const RCP_AxisData data);
    int processAMPressureData(const RCP_AMPressureData data);
    int processAMTemperatureData(const RCP_AMTemperatureData data);
    int processAccelerationData(const RCP_AxisData data);
    int processGyroData(const RCP_AxisData data);
    int processSerialData(const RCP_CustomData data);
}

#endif //RCP_HOST_IMPL_H
