#ifndef RCP_HOST_IMPL_H
#define RCP_HOST_IMPL_H

#include "RCP_Host/RCP_Host.h"

namespace LRI::RCI {
    extern RCP_LibInitData callbacks;
    size_t sendData(const void* data, size_t length);
    size_t readData(void* data, size_t bufferSize);
    int processTestUpdate(RCP_TestData data);
    int processSolenoidData(RCP_SolenoidData data);
    int processStepperData(RCP_StepperData data);
    int processTransducerData(RCP_TransducerData data);
    int processGPSData(RCP_GPSData data);
    int processMagnetometerData(RCP_AxisData data);
    int processAMPressureData(RCP_int32Data data);
    int processAMTemperatureData(RCP_int32Data data);
    int processAccelerationData(RCP_AxisData data);
    int processGyroData(RCP_AxisData data);
    int processSerialData(RCP_CustomData data);
    int processRelativeHumidityData(RCP_int32Data data);
}

#endif //RCP_HOST_IMPL_H
