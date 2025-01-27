#include "RCP.h"
#include <math.h>

void RCPWriteSerialString(const char* str);

#define RCPDebug(str)                                                          \
  static_assert(sizeof str < 64, "RCP Debug string too long!");                \
  RCPWriteSerialString(str "\n");

const float thermistor_measurement_in_celsius(int measurement) {
    constexpr float R1 = 10000.f;
    constexpr float B = 3950.f;
    constexpr float T0 = 273.15; // 0° Celsius in Kelvin
    constexpr float T25 = 25 + T0; // 25° Celsius in Kelvin
    constexpr float R25 = 5000.f;

    float Rt = R1 * (1023.f / measurement - 1); // Thermistor-Widerstand
    float temperature = 1.f / T25 + 1.f / B  * log(Rt/R25); // Kehrwert der Temperatur
    // in Kelvin
    temperature = 1.0 / temperature - T0; // Celsius
    return temperature;
}


struct Stepper {
    float pos = 0;
    float speed = 0;
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

float toFloat(const uint8_t* start) {
    return *((float*) start);
}

void setup() {
    Serial.begin(115200);
    while(!Serial);
    RCPDebug("setup");
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
    for(int i = 0; i < 8; i++)
        sols[i] = false;

    while(!Serial) {
        yield();
        delay(10);
    };
}

// void loop() {
//   delay(5000);
//   RCPDebug("Hello World");
// }

void loop() {
    // delay(500);
    timestamp = millis();

    if(datastreaming) {
        uint8_t data[10];
        data[0] = channel | 0x08;
        data[1] = 0x83;
        data[2] = timestamp >> 24;
        data[3] = timestamp >> 16;
        data[4] = timestamp >> 8;
        data[5] = timestamp;

        float temp = thermistor_measurement_in_celsius(analogRead(temppin));
        uint8_t* t = (uint8_t*) &temp;
        data[6] = t[0];
        data[7] = t[1];
        data[8] = t[2];
        data[9] = t[3];
        Serial.write(data, 10);
    }
    // display.clear();
    // char sbuf[20];
    // snprintf(sbuf, 20, "Avail: %d", Serial.available());
    // display.drawString(0, 10, sbuf);

    if(!hasheader) {
        digitalWrite(13, LOW);
        if(Serial.available()) {
            header = Serial.read();
            hasheader = true;
            RCPDebug("Recevied header");
        }
    }

    else {
        digitalWrite(13, HIGH);
        uint8_t length = (header & ~RCP_CHANNEL_MASK) + 1;
        if(Serial.available() >= length) {
            RCPDebug("Processing packet");
            uint8_t bytes[length] = {0};
            for(uint8_t i = 0; i < length; i++)
                bytes[i] = Serial.read();

            switch(bytes[0]) {
                case RCP_DEVCLASS_SOLENOID: {
                    uint8_t data[7] = {0};
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

                    if(op != 0) {
                        bool prev = sols[ID];
                        bool newb = false;
                        if(op == RCP_SOLENOID_ON)
                            newb = true;
                        else if(op == RCP_SOLENOID_TOGGLE)
                            newb = !prev;
                        // Serial.printf("Set sol to %d\n", (uint8_t) newb);
                        sols[ID] = newb;
                        if(timestamp % 57 == 0)
                            sols[ID] = prev;
                    }

                    data[6] = (sols[ID] ? RCP_SOLENOID_ON : RCP_SOLENOID_OFF) | ID;

                    Serial.write(data, 7);
                    break;
                }

                case RCP_DEVCLASS_STEPPER: {
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

                    if(op != RCP_STEPPER_QUERY_STATE) {
                        float val = toFloat(bytes + 2);
                        switch(op) {
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

                    uint8_t* pos = (uint8_t*) &steppers[id].pos;
                    uint8_t* speed = (uint8_t*) &steppers[id].speed;
                    pinMode(1, INPUT_PULLUP);

                    data[7] = pos[0];
                    data[8] = pos[1];
                    data[9] = pos[2];
                    data[10] = pos[3];
                    data[11] = speed[0];
                    data[12] = speed[1];
                    data[13] = speed[2];
                    data[14] = speed[3];
                    Serial.write(data, 15);
                    break;
                }

                case RCP_DEVCLASS_TEST_STATE: {
                    if((bytes[1] & 0xF0) == 0x20) {
                        datastreaming = (bytes[1]) == 0x21;
                    }

                    uint8_t data[7];
                    data[0] = channel | 0x05;
                    data[1] = 0x00;
                    data[2] = timestamp >> 24;
                    data[3] = timestamp >> 16;
                    data[4] = timestamp >> 8;
                    data[5] = timestamp;
                    data[6] = (datastreaming ? 0x80 : 0x00) | RCP_TEST_STOPPED;
                    Serial.write(data, 7);
                    break;
                }

                default:
                    break;
            }

            hasheader = false;
        }
    }
}

void RCPWriteSerialString(const char* str) {
    uint8_t len = strlen(str);
    if(len > 63)
        return;
    Serial.write(channel | len);
    Serial.write(RCP_DEVCLASS_CUSTOM);
    for(int i = 0; i < len; i++)
        Serial.write((uint8_t) str[i]);
}