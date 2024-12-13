#include "RCP_Host/RCP_Host.h"
#include "RCP_Host_Impl.h"

#include <UI/Steppers.h>


#include "UI/TargetChooser.h"
#include "UI/Solenoids.h"
#include "UI/TestControl.h"
#include "UI/SensorReadings.h"

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

    int processStepperData(const RCP_StepperData data) {
        Steppers::getInstance()->receiveRCPUpdate(data);
        return 0;
    }

    int processTransducerData(const RCP_TransducerData data) {
        SensorQualifier qual{.devclass = RCP_DEVCLASS_PRESSURE_TRANSDUCER, .id = data.ID};
        SensorReadings::getInstance()->receiveRCPUpdate(qual, {.timestamp = data.timestamp, .data = data.pressure});
        return 0;
    }

    constexpr SensorQualifier GPS_QUALIFIER{.devclass = RCP_DEVCLASS_GPS};

    int processGPSData(const RCP_GPSData data) {
        DataPoint d{.timestamp = data.timestamp};
        memcpy(&d.data, &data.latitude, 32);
        SensorReadings::getInstance()->receiveRCPUpdate(GPS_QUALIFIER, d);
        return 0;
    }

    constexpr SensorQualifier MAGNETOMETER_QUALIFIER{.devclass = RCP_DEVCLASS_MAGNETOMETER};

    int processMagnetometerData(const RCP_AxisData data) {
        DataPoint d{.timestamp = data.timestamp};
        memcpy(&d.data, &data.x, 12);
        SensorReadings::getInstance()->receiveRCPUpdate(MAGNETOMETER_QUALIFIER, d);
        return 0;
    }

    constexpr SensorQualifier AM_PRESSURE_QUALIFIER{.devclass = RCP_DEVCLASS_AM_PRESSURE};

    int processAMPressureData(const RCP_AMPressureData data) {
        SensorReadings::getInstance()->receiveRCPUpdate(AM_PRESSURE_QUALIFIER,
                                                        {.timestamp = data.timestamp, .data = data.pressure});
        return 0;
    }

    constexpr SensorQualifier AM_TEMPERATURE_QUALIFIER{.devclass = RCP_DEVCLASS_AM_TEMPERATURE};

    int processAMTemperatureData(const RCP_AMTemperatureData data) {
        SensorReadings::getInstance()->receiveRCPUpdate(AM_TEMPERATURE_QUALIFIER,
                                                        {.timestamp = data.timestamp, .data = data.temperature});
        return 0;
    }

    constexpr SensorQualifier ACCELEROMETER_QUALIFIER{.devclass = RCP_DEVCLASS_ACCELEROMETER};
    int processAccelerationData(const RCP_AxisData data) {
        DataPoint d{.timestamp = data.timestamp};
        memcpy(&d.data, &data.x, 12);
        SensorReadings::getInstance()->receiveRCPUpdate(ACCELEROMETER_QUALIFIER, d);
        return 0;
    }

    constexpr SensorQualifier GYRO_QUALIFIER{.devclass = RCP_DEVCLASS_GYROSCOPE};
    int processGyroData(const RCP_AxisData data) {
        DataPoint d{.timestamp = data.timestamp};
        memcpy(&d.data, &data.x, 12);
        SensorReadings::getInstance()->receiveRCPUpdate(GYRO_QUALIFIER, d);
        return 0;
    }

    int processSerialData(const RCP_CustomData data) {
        return 0;
    }
}
