#include "RCP.h"

void setup() {
    Serial.begin(115200);
    while(!Serial);

    RCPDebug("setup");
    RCP::setReady(true);
}

void loop() {
    // delay(500);
    RCP::yield();
    RCP::runTest();   
}