#include "RCP.h"

void setup() {
    Serial.begin(115200);
    while(!Serial) {
        yield();
        delay(10);
    }

    RCP::init();
    RCP::setReady(true);
    RCPDebug("[RCP] Initialization Complete");
}

void loop() {
    RCP::yield();
    RCP::runTest();   
}