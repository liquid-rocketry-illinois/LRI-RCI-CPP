#include "RCP.h"
#include "test.h"
#include <Arduino.h>

namespace RCP {

RCP_Channel_t channel;
LRI::RingBuf<uint8_t, SERIAL_BUFFER_SIZE> inbuffer;

bool dataStreaming;
uint8_t testNum;
RCP_TestRunningState_t testState;
bool ready;

bool firstTestRun;
bool initDone = false;

uint8_t pval[4];
RCP_PromptDataType_t lastType;
PromptAcceptor pacceptor;

void init() {
  testNum = 0;
  testState = RCP_TEST_STOPPED;
  dataStreaming = false;
  firstTestRun = false;
  initDone = true;
}

void setReady(bool newready) {
  if(!initDone || newready == ready)
    return;
  ready = newready;
  sendTestStateRCP();
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

void sendTestStateRCP() {
  uint8_t data[7] = {0};
  data[0] = channel | 0x05;
  data[6] = testState | testNum | (dataStreaming ? 0x80 : 0x00) |
            (ready ? 0x10 : 0x00);
  Serial.write(data, 7);
}

void ESTOP() {
  if(testState == RCP_TEST_RUNNING || testState == RCP_TEST_PAUSED)
    Test::tests[testNum].end(true);
  testState = RCP_TEST_ESTOP;
  sendTestStateRCP();
  RCPDebug("estop");
  while(true) {
  }
}

void setPrompt(const char* str, RCP_PromptDataType_t gng, PromptAcceptor acceptor) {
  size_t len = strlen(str);
  if(len > 63) return;
  pacceptor = acceptor;
  lastType = gng;
  uint8_t pkt[66];
  pkt[0] = channel | (len + 1);
  pkt[1] = RCP_DEVCLASS_PROMPT;
  pkt[2] = gng;
  memcpy(pkt + 3, str, len);
  Serial.write(pkt, len + 3);
}

// The majority of RCP related functions
void yield() {
  if(!initDone)
    return;
  // Read SERIAL_BYTES_PER_LOOP bytes into the buffer
  for(int i = 0; i < SERIAL_BYTES_PER_LOOP && Serial.available(); i++) {
    uint8_t val = Serial.read();
    inbuffer.pushOverwrite(val);
  }

  // Calculate the packet length from the header available in the buffer
  if(inbuffer.isEmpty())
    return;
  uint8_t head = 0;
  inbuffer.peek(head, 0);
  uint8_t pktlen = head & (~RCP_CHANNEL_MASK);

  // RCP::RCPWriteSerialString((String(head) + " " + String(inbuffer.size()) +
  // "\n").c_str()); If the packet length is zero, this indicates an ESTOP
  // condition. Do that immediately.
  if(pktlen == 0)
    ESTOP();

  // If the buffer contains all the bytes for a packet, read it in to the
  // array bytes
  if(inbuffer.size() >= pktlen + 2) {
    // RCPDebug("pkt");
    uint8_t bytes[pktlen + 2];
    for(int i = 0; i < pktlen + 2; i++) {
      inbuffer.pop(bytes[i]);
    }

    // If the channel does not match, exit early
    if((bytes[0] & RCP_CHANNEL_MASK) != channel)
      return;

    // Switch on the device class
    switch(bytes[1]) {
      // Handle test state packet
    case RCP_DEVCLASS_TEST_STATE: {
      switch(bytes[2] & 0xF0) {
      case 0x00:
        if(testState != RCP_TEST_STOPPED)
          break;
        testNum = bytes[2] & 0x0F;
        testState = RCP_TEST_RUNNING;
        firstTestRun = true;
        break;

      case 0x10:
        if((bytes[2] & 0x0F) == 0) {
          if(testState == RCP_TEST_RUNNING || testState == RCP_TEST_PAUSED) {
            Test::tests[testNum].end(true);
            testState = RCP_TEST_STOPPED;
          }
        }

        else
          testState = (testState == RCP_TEST_RUNNING ? RCP_TEST_PAUSE
                                                     : RCP_TEST_RUNNING);
        break;

      case 0x20:
        dataStreaming = (bytes[2] & 0x0F) != 0;
        break;

      default:
        break;
      }

      sendTestStateRCP();

      break;
    }

    case RCP_DEVCLASS_PROMPT: {
      if(!pacceptor) break;
      if(lastType == RCP_PromptDataType_GONOGO) pval[0] = bytes[2];
      else memcpy(pval, bytes + 2, 4);
      pacceptor(pval);
      pacceptor = nullptr;
      break;
    }

    default:
      break;
    }
  }
}

void runTest() {
  if(testState != RCP_TEST_RUNNING)
    return;
  Test::Procedure& test = Test::tests[testNum];

  if(firstTestRun) {
    test.initialize();
    firstTestRun = false;
  }

  test.execute();
  if(test.isFinished()) {
    test.end(false);
    testState = RCP_TEST_STOPPED;
    firstTestRun = true;
    sendTestStateRCP();
  }
}

} // namespace RCP