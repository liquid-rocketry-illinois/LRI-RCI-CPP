#ifndef RCP_HOST_H
#define RCP_HOST_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef uint8_t RCP_Channel_t;

enum RCP_Channel {
    RCP_CH_ZERO = 0x00,
    RCP_CH_ONE = 0x40,
    RCP_CH_TWO = 0x80,
    RCP_CH_THREE = 0xC0,
    RCP_CHANNEL_MASK = 0xC0,
};

typedef uint8_t RCP_DeviceClass_t;

enum RCP_DeviceClass {
    RCP_DEVCLASS_TEST_STATE = 0x00,
    RCP_DEVCLASS_SOLENOID = 0x01,
    RCP_DEVCLASS_STEPPER = 0x02,
    RCP_DEVCLASS_CUSTOM = 0x80,

    RCP_DEVCLASS_AM_PRESSURE = 0x90,
    RCP_DEVCLASS_AM_TEMPERATURE = 0x91,
    RCP_DEVCLASS_PRESSURE_TRANSDUCER = 0x92,
    RCP_DEVCLASS_RELATIVE_HYGROMETER = 0x93,
    RCP_DEVCLASS_LOAD_CELL = 0x94,

    RCP_DEVCLASS_POWERMON = 0xA0,

    RCP_DEVCLASS_ACCELEROMETER = 0xB0,
    RCP_DEVCLASS_GYROSCOPE = 0xB1,
    RCP_DEVCLASS_MAGNETOMETER = 0xB2,

    RCP_DEVCLASS_GPS = 0xC0,
};

typedef uint8_t RCP_TestStateControlMode_t;

enum RCP_TestStateControlMode {
    RCP_TEST_START = 0x00,
    RCP_TEST_STOP = 0x10,
    RCP_TEST_PAUSE = 0x11,
    RCP_DATA_STREAM_STOP = 0x20,
    RCP_DATA_STREAM_START = 0x21,
    RCP_TEST_QUERY = 0x30,
    RCP_HEARTBEATS_CONTROL = 0xF0
};

typedef uint8_t RCP_TestRunningState_t;

enum RCP_TestRunningState {
    RCP_TEST_RUNNING = 0x00,
    RCP_TEST_STOPPED = 0x20,
    RCP_TEST_PAUSED = 0x40,
    RCP_TEST_ESTOP = 0x60,
    RCP_TEST_STATE_MASK = 0x60,
};

typedef uint8_t RCP_SolenoidState_t;

enum RCP_SolenoidState {
    RCP_SOLENOID_READ = 0x00,
    RCP_SOLENOID_ON = 0x40,
    RCP_SOLENOID_OFF = 0x80,
    RCP_SOLENOID_TOGGLE = 0xC0,
    RCP_SOLENOID_STATE_MASK = 0xC0,
};

typedef uint8_t RCP_StepperControlMode_t;

enum RCP_StepperControlMode {
    RCP_STEPPER_QUERY_STATE = 0x00,
    RCP_STEPPER_ABSOLUTE_POS_CONTROL = 0x40,
    RCP_STEPPER_RELATIVE_POS_CONTROL = 0x80,
    RCP_STEPPER_SPEED_CONTROL = 0xC0,
    RCP_STEPPER_CONTROL_MODE_MASK = 0xC0
};

struct RCP_TestData {
    uint32_t timestamp;
    int dataStreaming;
    RCP_TestRunningState_t state;
    uint8_t heartbeatTime;
};

struct RCP_SolenoidData {
    uint32_t timestamp;
    RCP_SolenoidState_t state;
    uint8_t ID;
};

struct RCP_OneFloat {
    RCP_DeviceClass_t devclass;
    uint32_t timestamp;
    uint8_t ID;
    float data;
};

struct RCP_TwoFloat {
    RCP_DeviceClass_t devclass;
    uint32_t timestamp;
    uint8_t ID;
    float data[2];
};

struct RCP_ThreeFloat {
    RCP_DeviceClass_t devclass;
    uint32_t timestamp;
    uint8_t ID;
    float data[3];
};

struct RCP_FourFloat {
    RCP_DeviceClass_t devclass;
    uint32_t timestamp;
    uint8_t ID;
    float data[4];
};

struct RCP_CustomData {
    void* data;
    uint8_t length;
};

#ifdef __cplusplus
}
#endif

#endif //RCP_HOST_H
