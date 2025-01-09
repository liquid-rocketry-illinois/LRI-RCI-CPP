// #include <HT_SSD1306Wire.h>
#include "RCP.h"
// static SSD1306Wire  display(0x3c, 500000, SDA_OLED, SCL_OLED, GEOMETRY_128_64, RST_OLED);

#include <math.h>

const int32_t thermistor_measurement_in_celsius(int measurement) {
  const float R1 = 10000.f;
  const float B = 3950.f;
  const float T0 = 273.15;    // 0° Celsius in Kelvin
  const float T25 = 25 + T0;  // 25° Celsius in Kelvin
  const float R25 = 5000.f;

  double Rt = R1 * (1023.f / measurement - 1);               // Thermistor-Widerstand
  double temperature = 1.f / T25 + 1.f / B * log(Rt / R25);  // Kehrwert der Temperatur
                                                             // in Kelvin
  temperature = 1.0 / temperature - T0;                      // Celsius
  return (int32_t)(temperature * 1000000);
}

struct Stepper {
  int32_t pos = 0;
  int32_t speed = 0;
};

constexpr int temppin = A0;

bool datastreaming;
uint32_t timestamp = 0;
static uint8_t header;
static bool hasheader;
static uint8_t channel;
static bool sols[64];
static Stepper steppers[64];
static int failure;

int32_t toInt32(const uint8_t* start) {
  return (((int32_t)start[0]) << 24) | (((int32_t)start[1]) << 16) | (((int32_t)start[2]) << 8) | ((int32_t)start[3]);
}

void setup() {
  Serial.begin(115200);
  // Serial.setRxBufferSize(1024);
  // display.init();
  // display.clear();
  // display.display();
  timestamp = 0;
  header = 0;
  hasheader = false;
  channel = RCP_CH_ZERO;
  failure = 0;
  datastreaming = false;

  pinMode(13, OUTPUT);
  for (int i = 0; i < 8; i++) sols[i] = false;
}

void loop() {
  // delay(500);
  delay(1);
  timestamp = millis();

  if (datastreaming) {
    uint8_t data[10];
    data[0] = channel | 0x08;
    data[1] = 0x83;
    data[2] = timestamp >> 24;
    data[3] = timestamp >> 16;
    data[4] = timestamp >> 8;
    data[5] = timestamp;

    int32_t temp = thermistor_measurement_in_celsius(analogRead(temppin));
    data[6] = temp >> 24;
    data[7] = temp >> 16;
    data[8] = temp >> 8;
    data[9] = temp;
    Serial.write(data, 10);
  }
  // display.clear();
  // char sbuf[20];
  // snprintf(sbuf, 20, "Avail: %d", Serial.available());
  // display.drawString(0, 10, sbuf);

  if (!hasheader) {
    digitalWrite(13, LOW);
    if (Serial.available()) {
      header = Serial.read();
      hasheader = true;
    }
  }

  else {
    digitalWrite(13, HIGH);
    uint8_t length = (header & ~RCP_CHANNEL_MASK) + 1;
    if (Serial.available() >= length) {
      uint8_t bytes[length] = { 0 };
      for (uint8_t i = 0; i < length; i++) bytes[i] = Serial.read();

      switch (bytes[0]) {
        case RCP_DEVCLASS_SOLENOID:
          {
            uint8_t data[7] = { 0 };
            data[0] = channel | 0x05;
            data[1] = RCP_DEVCLASS_SOLENOID;
            data[2] = timestamp >> 24;
            data[3] = timestamp >> 16;
            data[4] = timestamp >> 8;
            data[5] = timestamp;
            uint8_t op = bytes[1] & RCP_SOLENOID_STATE_MASK;
            uint8_t ID = bytes[1] & 0x3F;

            // char buf[20];
            // snprintf(buf, 20, "Processing ID %d", ID);
            // Serial.println(buf);
            // display.drawString(0, 0, buf);


            if (op != 0) {
              bool prev = sols[ID];
              bool newb = false;
              if (op == RCP_SOLENOID_ON) newb = true;
              else if (op == RCP_SOLENOID_TOGGLE) newb = !prev;
              // Serial.printf("Set sol to %d\n", (uint8_t) newb);
              sols[ID] = newb;
              if (timestamp % 57 == 0) sols[ID] = prev;
            }

            data[6] = (sols[ID] ? RCP_SOLENOID_ON : RCP_SOLENOID_OFF) | ID;

            Serial.write(data, 7);
            break;
          }

        case RCP_DEVCLASS_STEPPER:
          {
            uint8_t data[15];
            data[0] = channel | 0x0D;
            data[1] = RCP_DEVCLASS_STEPPER;
            data[2] = timestamp >> 24;
            data[3] = timestamp >> 16;
            data[4] = timestamp >> 8;
            data[5] = timestamp;

            uint8_t op = bytes[1] & RCP_STEPPER_CONTROL_MODE_MASK;
            uint8_t id = bytes[1] & ~RCP_STEPPER_CONTROL_MODE_MASK;
            data[6] = id;

            if (op != RCP_STEPPER_QUERY_STATE) {
              int32_t val = toInt32(bytes + 2);
              switch (op) {
                case RCP_STEPPER_SPEED_CONTROL:
                  steppers[id].speed = val;
                  break;

                case RCP_STEPPER_ABSOLUTE_POS_CONTROL:
                  steppers[id].pos = val;
                  break;

                case RCP_STEPPER_RELATIVE_POS_CONTROL:
                  steppers[id].pos += val;
                  break;
              }
            }

            int32_t pos = steppers[id].pos;
            int32_t speed = steppers[id].speed;
            pinMode(1, INPUT_PULLUP);

            data[7] = pos >> 24;
            data[8] = pos >> 16;
            data[9] = pos >> 8;
            data[10] = pos;
            data[11] = speed >> 24;
            data[12] = speed >> 16;
            data[13] = speed >> 8;
            data[14] = speed;
            Serial.write(data, 15);
            break;
          }

        case RCP_DEVCLASS_TEST_STATE:
          {
            if ((bytes[1] & 0xF0) == 0x20) {
              datastreaming = (bytes[1]) == 0x21;
            }

            uint8_t data[7];
            data[0] = channel | 0x05;
            data[1] = 0x00;
            data[2] = timestamp >> 24;
            data[3] = timestamp >> 16;
            data[4] = timestamp >> 8;
            data[5] = timestamp;
            data[6] = (datastreaming ? 0x80 : 0x00);
            Serial.write(data, 7);
            break;
          }

        default:
          break;
      }

      hasheader = false;
    }
  }
  // display.display();
  // timestamp++;
  // if(Serial.available() >= 3) {
  //   int byte = Serial.read();
  //   if(byte & 0x3F != 2) return;
  //   byte = Serial.read();
  //   if(byte != 0x81) return;
  //   byte = Serial.read();
  //   int id = byte & 0x3F;
  //   Serial.write(0x06);
  //   Serial.write(0x01);
  //   Serial.write(timestamp >> 24);
  //   Serial.write(timestamp >> 16);
  //   Serial.write(timestamp >> 8);
  //   Serial.write(timestamp);
  //   Serial.write(id | (id % 2 == 0 ? 0x40 : 0x00));
  // }
}
