#include "Arduino.h"
#include "NRF24L01P.h"

#define SS_PIN 10
#define CE_PIN 11


enum STATE {
    BINDING,
    RUNNING
};

Radio radio(10, 11);

STATE currentState;

void setup() {
    currentState = STATE::BINDING;
    // setting up environment
};

void loop() {
    switch (currentState) {
        case STATE::BINDING: {
            radio.ping_master();
            radio.start_listening();
            delay(5);
            if (radio.available()) {

            }
            break;
        }
        case STATE::RUNNING: {
            break;
        }
        default: break;
    }
};
