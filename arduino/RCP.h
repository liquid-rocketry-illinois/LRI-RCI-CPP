enum RCP_Channel {
  RCP_CH_ZERO = 0x00,
  RCP_CH_ONE = 0x40,
  RCP_CH_TWO = 0x80,
  RCP_CH_THREE = 0xC0,
  RCP_CHANNEL_MASK = 0xC0,
};

enum RCP_DeviceClass {
  RCP_DEVCLASS_TEST_STATE = 0x00,
  RCP_DEVCLASS_SOLENOID = 0x01,
  RCP_DEVCLASS_STEPPER = 0x02,
  RCP_DEVCLASS_CUSTOM = 0x80,
  RCP_DEVCLASS_GPS = 0x81,
  RCP_DEVCLASS_AM_PRESSURE = 0x82,
  RCP_DEVCLASS_AM_TEMPERATURE = 0x83,
  RCP_DEVCLASS_ACCELEROMETER = 0x84,
  RCP_DEVCLASS_GYROSCOPE = 0x85,
  RCP_DEVCLASS_MAGNETOMETER = 0x86,
  RCP_DEVCLASS_PRESSURE_TRANSDUCER = 0x87,
};

enum RCP_TestStateControlMode {
  RCP_TEST_START = 0x00,
  RCP_TEST_STOP = 0x10,
  RCP_TEST_PAUSE = 0x11,
  RCP_DATA_STREAM_START = 0x20,
  RCP_DATA_STREAM_STOP = 0x21,
  RCP_TEST_QUERY = 0x30,
  RCP_HEARTBEATS_CONTROL = 0xF0
};

enum RCP_TestRunningState {
  RCP_TEST_RUNNING = 0x00,
  RCP_TEST_STOPPED = 0x20,
  RCP_TEST_PAUSED = 0x40,
  RCP_TEST_ESTOP = 0x60,
  RCP_TEST_STATE_MASK = 0x60,
};

enum RCP_SolenoidState {
  RCP_SOLENOID_READ = 0x00,
  RCP_SOLENOID_ON = 0x40,
  RCP_SOLENOID_OFF = 0x80,
  RCP_SOLENOID_TOGGLE = 0xC0,
  RCP_SOLENOID_STATE_MASK = 0xC0,
};

enum RCP_StepperControlMode {
  RCP_STEPPER_QUERY_STATE = 0x00,
  RCP_STEPPER_ABSOLUTE_POS_CONTROL = 0x40,
  RCP_STEPPER_RELATIVE_POS_CONTROL = 0x80,
  RCP_STEPPER_SPEED_CONTROL = 0xC0,
  RCP_STEPPER_CONTROL_MODE_MASK = 0xC0
};

enum DataStreamingState {
  RCP_DATA_STREAMING_ENABLED = 0x80,
  RCP_DATA_STREAMING_DISABLED = 0x00
};

struct RCP_TestData {
  int32_t timestamp;
  uint8_t dataStreaming;
  RCP_TestRunningState state;
  uint8_t heartbeatTime;
};

struct RCP_SolenoidData {
  int32_t timestamp;
  RCP_SolenoidState state;
  uint8_t ID;
};

struct RCP_StepperData {
  int32_t timestamp;
  uint8_t ID;
  int32_t position;
  int32_t speed;
};

struct RCP_TransducerData {
  int32_t timestamp;
  uint8_t ID;
  int32_t pressure;
};

struct RCP_GPSData {
  int32_t timestamp;
  int64_t latitude;
  int64_t longitude;
  int64_t altitude;
  int64_t groundSpeed;
};

struct RCP_AxisData {
  int32_t timestamp;
  int32_t x;
  int32_t y;
  int32_t z;
};

struct RCP_AMPressureData {
  int32_t timestamp;
  int32_t pressure;
};

struct RCP_AMTemperatureData {
  int32_t timestamp;
  int32_t temperature;
};

struct RCP_CustomData {
  int32_t timestamp;
  uint8_t length;
  void* data;
};