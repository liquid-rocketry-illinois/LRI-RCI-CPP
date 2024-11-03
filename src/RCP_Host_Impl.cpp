#include "RCP_Host/RCP_Host.h"

namespace LRI::RCP {
    size_t sendData(const void* data, size_t length) {
        return 0;
    }

    size_t readData(const void* data, size_t bufferSize) {
        return 0;
    }

    int processTestUpdate(const struct TestData data) {
        return 0;
    }

    int processSolenoidData(const struct SolenoidData data) {
        return 0;
    }

    int processStepperData(const struct StepperData data) {
        return 0;
    }

    int processTransducerData(const struct TransducerData data) {
        return 0;
    }

    int processGPSData(const struct GPSData data) {
        return 0;
    }

    int processMagnetometerData(const struct AxisData data) {
        return 0;
    }

    int processAMPressureData(const struct AMPressureData data) {
        return 0;
    }

    int processAMTemperatureData(const struct AMTemperatureData data) {
        return 0;
    }

    int processAccelerationData(const struct AxisData data) {
        return 0;
    }

    int processGyroData(const struct AxisData data) {
        return 0;
    }

    int processSerialData(const struct SerialData data) {
        return 0;
    }
}
