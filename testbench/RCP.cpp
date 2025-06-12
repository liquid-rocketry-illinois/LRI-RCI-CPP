#include <Arduino.h>

#include "RCP.h"

#include "test.h"

namespace RCP {

    RCP_Channel channel;
    LRI::RingBuf<uint8_t, RCP_SERIAL_BUFFER_SIZE> inbuffer;

    Test::Procedure* ESTOP_PROC = new Test::Procedure();

    uint8_t testNum;
    RCP_TestRunningState testState;
    bool dataStreaming;
    bool ready = false;

    bool firstTestRun;
    bool initDone = false;
    uint32_t timeOffset = 0;

    uint8_t promptValue[4];
    RCP_PromptDataType lastType;
    PromptAcceptor pacceptor;

    void init() {
        testNum = 0;
        testState = RCP_TEST_STOPPED;
        dataStreaming = false;
        firstTestRun = false;
        initDone = true;
    }

    void setReady(bool newready) {
        if(!initDone || newready == ready) return;
        ready = newready;
        sendTestState();
    }

    void sendTestState() {
        uint8_t data[7] = {0};
        data[0] = channel | 0x05;
        data[6] = testState | testNum | (dataStreaming ? 0x80 : 0x00) | (ready ? 0x10 : 0x00);
        Serial.write(data, 7);
    }

    void RCPWriteSerialString(const char* str) {
        uint8_t len = strlen(str);
        if(len > 63) return;
        uint8_t data[65] = {0};
        data[0] = channel | len;
        data[1] = RCP_DEVCLASS_CUSTOM;
        memcpy(data + 2, str, len);
        Serial.write(data, len + 2);
    }

    void setPrompt(const char* str, RCP_PromptDataType gng, PromptAcceptor acceptor) {
        size_t len = strlen(str);
        if(len > 62) return;
        pacceptor = acceptor;
        lastType = gng;
        uint8_t pkt[65];
        pkt[0] = channel | (len + 1);
        pkt[1] = RCP_DEVCLASS_PROMPT;
        pkt[2] = gng;
        memcpy(pkt + 3, str, len);
        Serial.write(pkt, len + 3);
    }

    void resetPrompt() {
        pacceptor = nullptr;
        uint8_t pkt[3] = {0};
        pkt[0] = channel | 1;
        pkt[1] = RCP_DEVCLASS_PROMPT;
        pkt[2] = RCP_PromptDataType_RESET;
        Serial.write(pkt, 3);
    }

    void ESTOP() {
        if(testState == RCP_TEST_RUNNING || testState == RCP_TEST_PAUSED) Test::tests[testNum]->end(true);
        testState = RCP_TEST_ESTOP;
        sendTestState();
        ESTOP_PROC->initialize();
        while(!ESTOP_PROC->isFinished()) ESTOP_PROC->execute();
        ESTOP_PROC->end(false);
        while(true) {}
    }

    // The majority of RCP related functions
    void yield() {
        if(!initDone) return;
        // Read SERIAL_BYTES_PER_LOOP bytes into the buffer
        for(int i = 0; i < SERIAL_BYTES_PER_LOOP && Serial.available(); i++) {
            uint8_t val = Serial.read();
            inbuffer.pushOverwrite(val);
        }

        // Calculate the packet length from the header available in the buffer
        if(inbuffer.isEmpty()) return;
        uint8_t head = 0;
        inbuffer.peek(head, 0);
        uint8_t pktlen = head & (~RCP_CHANNEL_MASK);

        // RCP::RCPWriteSerialString((String(head) + " " + String(inbuffer.size()) + "\n").c_str());
        // If the packet length is zero, this indicates an ESTOP condition. Do that immediately.
        if(pktlen == 0) ESTOP();

        // If the buffer contains all the bytes for a packet, read it in to the
        // array bytes
        if(inbuffer.size() >= pktlen + 2) {
            // RCPDebug("pkt");
            uint8_t bytes[pktlen + 2];
            for(int i = 0; i < pktlen + 2; i++) {
                inbuffer.pop(bytes[i]);
            }

            // If the channel does not match, exit early
            if((bytes[0] & RCP_CHANNEL_MASK) != channel) return;

            // Switch on the device class
            switch(bytes[1]) {
                // Handle test state packet
            case RCP_DEVCLASS_TEST_STATE: {
                switch(bytes[2] & 0xF0) {
                case 0x00:
                    if(testState != RCP_TEST_STOPPED) break;
                    testNum = bytes[2] & 0x0F;
                    testState = RCP_TEST_RUNNING;
                    firstTestRun = true;
                    break;

                case 0x10: {
                    switch(bytes[2] & 0x0F) {
                    case 0x00:
                        if(testState == RCP_TEST_RUNNING || testState == RCP_TEST_PAUSED) {
                            Test::tests[testNum]->end(true);
                            testState = RCP_TEST_STOPPED;
                            resetPrompt();
                        }
                        break;

                    case 0x01: {
                        if(testState == RCP_TEST_RUNNING)
                            testState = RCP_TEST_PAUSED;
                        else if(testState == RCP_TEST_PAUSED)
                            testState = RCP_TEST_RUNNING;
                        break;
                    }

                    case 0x02:
                        systemReset();
                        break;

                    case 0x03:
                        timeOffset = millis();
                        break;
                    }

                    break;
                }

                case 0x20:
                    dataStreaming = (bytes[2] & 0x0F) != 0;
                    break;

                default:
                    break;
                }

                sendTestState();

                break;
            }

            case RCP_DEVCLASS_PROMPT: {
                if(!pacceptor) break;
                if(lastType == RCP_PromptDataType_GONOGO)
                    promptValue[0] = bytes[2];
                else
                    memcpy(promptValue, bytes + 2, 4);
                pacceptor(promptValue);
                pacceptor = nullptr;
                break;
            }

            default:
                break;
            }
        }
    }

    void runTest() {
        if(testState != RCP_TEST_RUNNING) return;
        Test::Procedure* test = Test::tests[testNum];

        if(firstTestRun) {
            test->initialize();
            firstTestRun = false;
        }

        test->execute();
        if(test->isFinished()) {
            test->end(false);
            testState = RCP_TEST_STOPPED;
            firstTestRun = true;
            sendTestState();
        }
    }

    __attribute__((weak)) void systemReset() {
        while(true) {}
    }

    void sendOneFloat(const RCP_DeviceClass devclass, const uint8_t id, const float* value) {
        uint32_t time = millis() - timeOffset;
        uint8_t data[11] = {0};
        data[0] = channel | 11;
        data[1] = devclass;
        data[2] = time >> 24;
        data[3] = time >> 16;
        data[4] = time >> 8;
        data[5] = time;
        data[6] = id;
        memcpy(data + 7, value, 4);
        Serial.write(data, 11);
    }

    void sendTwoFloat(const RCP_DeviceClass devclass, const uint8_t id, const float value[2]) {
        uint32_t time = millis() - timeOffset;
        uint8_t data[15] = {0};
        data[0] = channel | 15;
        data[1] = devclass;
        data[2] = time >> 24;
        data[3] = time >> 16;
        data[4] = time >> 8;
        data[5] = time;
        data[6] = id;
        memcpy(data + 7, value, 8);
        Serial.write(data, 15);
    }

    void sendThreeFloat(const RCP_DeviceClass devclass, const uint8_t id, const float value[3]) {
        uint32_t time = millis() - timeOffset;
        uint8_t data[19] = {0};
        data[0] = channel | 19;
        data[1] = devclass;
        data[2] = time >> 24;
        data[3] = time >> 16;
        data[4] = time >> 8;
        data[5] = time;
        data[6] = id;
        memcpy(data + 7, value, 12);
        Serial.write(data, 19);
    }

    void sendFourFloat(const RCP_DeviceClass devclass, const uint8_t id, const float value[4]) {
        uint32_t time = millis() - timeOffset;
        uint8_t data[23] = {0};
        data[0] = channel | 23;
        data[1] = devclass;
        data[2] = time >> 24;
        data[3] = time >> 16;
        data[4] = time >> 8;
        data[5] = time;
        data[6] = id;
        memcpy(data + 7, value, 16);
        Serial.write(data, 23);
    }
} // namespace RCP
