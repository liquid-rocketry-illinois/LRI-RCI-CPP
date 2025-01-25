#include "RCP_Host/RCP_Host.h"
#include "RCP_Host_Impl.h"

#include <UI/Steppers.h>

#include "UI/TargetChooser.h"
#include "UI/Solenoids.h"
#include "UI/TestControl.h"
#include "UI/SensorReadings.h"
#include "UI/CustomData.h"

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
        .processHumidityData = processRelativeHumidityData,
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
        SensorQualifier qual(RCP_DEVCLASS_PRESSURE_TRANSDUCER, data.ID);
        SensorReadings::getInstance()->receiveRCPUpdate(qual, {
                                                            .timestamp = static_cast<double>(data.timestamp) / 1'000.0,
                                                            .data = {static_cast<double>(data.pressure)}});
        return 0;
    }

    int processGPSData(const RCP_GPSData data) {
        static const SensorQualifier GPS_QUALIFIER{.devclass = RCP_DEVCLASS_GPS};
        DataPoint d{.timestamp = static_cast<double>(data.timestamp) / 1'000.0};
        d.data.gpsData[0] = static_cast<double>(data.latitude);
        d.data.gpsData[1] = static_cast<double>(data.longitude);
        d.data.gpsData[2] = static_cast<double>(data.altitude);
        d.data.gpsData[3] = static_cast<double>(data.groundSpeed);
        SensorReadings::getInstance()->receiveRCPUpdate(GPS_QUALIFIER, d);
        return 0;
    }

    int processMagnetometerData(const RCP_AxisData data) {
        static const SensorQualifier MAGNETOMETER_QUALIFIER{.devclass = RCP_DEVCLASS_MAGNETOMETER};
        DataPoint d{.timestamp = static_cast<double>(data.timestamp) / 1'000.0};
        d.data.axisData[0] = static_cast<double>(data.x);
        d.data.axisData[1] = static_cast<double>(data.y);
        d.data.axisData[2] = static_cast<double>(data.z);
        SensorReadings::getInstance()->receiveRCPUpdate(MAGNETOMETER_QUALIFIER, d);
        return 0;
    }

    int processAMPressureData(const RCP_floatData data) {
        static const SensorQualifier AM_PRESSURE_QUALIFIER{.devclass = RCP_DEVCLASS_AM_PRESSURE};
        SensorReadings::getInstance()->receiveRCPUpdate(AM_PRESSURE_QUALIFIER, {
                                                            .timestamp = static_cast<double>(data.timestamp) / 1'000.0,
                                                            .data = {static_cast<double>(data.data)}});
        return 0;
    }

    int processAMTemperatureData(const RCP_floatData data) {
        static const SensorQualifier AM_TEMPERATURE_QUALIFIER{.devclass = RCP_DEVCLASS_AM_TEMPERATURE};
        SensorReadings::getInstance()->receiveRCPUpdate(AM_TEMPERATURE_QUALIFIER, {
                                                            .timestamp = static_cast<double>(data.timestamp) / 1'000.0,
                                                            .data = {static_cast<double>(data.data)}});
        return 0;
    }

    int processAccelerationData(const RCP_AxisData data) {
        static const SensorQualifier ACCELEROMETER_QUALIFIER{.devclass = RCP_DEVCLASS_ACCELEROMETER};
        DataPoint d{.timestamp = static_cast<double>(data.timestamp) / 1'000.0};
        d.data.axisData[0] = static_cast<double>(data.x);
        d.data.axisData[1] = static_cast<double>(data.y);
        d.data.axisData[2] = static_cast<double>(data.z);
        SensorReadings::getInstance()->receiveRCPUpdate(ACCELEROMETER_QUALIFIER, d);
        return 0;
    }

    int processGyroData(const RCP_AxisData data) {
        static const SensorQualifier GYRO_QUALIFIER{.devclass = RCP_DEVCLASS_GYROSCOPE};
        DataPoint d{.timestamp = static_cast<double>(data.timestamp) / 1'000.0};
        d.data.axisData[0] = static_cast<double>(data.x);
        d.data.axisData[1] = static_cast<double>(data.y);
        d.data.axisData[2] = static_cast<double>(data.z);
        SensorReadings::getInstance()->receiveRCPUpdate(GYRO_QUALIFIER, d);
        return 0;
    }

    int processRelativeHumidityData(RCP_floatData data) {
        static const SensorQualifier RELHUMIDITY_QUALIFIER{.devclass = RCP_DEVCLASS_RELATIVE_HYGROMETER};
        SensorReadings::getInstance()->receiveRCPUpdate(RELHUMIDITY_QUALIFIER, {
                                                            .timestamp = static_cast<double>(data.timestamp) / 1'000.0,
                                                            .data = {static_cast<double>(data.data)}});
        return 0;
    }


    int processSerialData(const RCP_CustomData data) {
        CustomData::getInstance()->recevieRCPUpdate(data);
        return 0;
    }
}
