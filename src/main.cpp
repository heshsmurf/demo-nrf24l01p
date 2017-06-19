#include "Arduino.h"
#include "NRF24L01P.h"

#define SS_PIN 6
#define CE_PIN 7
#define SWITCH_PIN 8


enum STATE {
    BINDING,
    RUNNING
};

Radio radio(SS_PIN, CE_PIN);

STATE currentState;

bool state = false;

bool isRX = false;

void setup() {
	pinMode(SS_PIN, OUTPUT);
	pinMode(CE_PIN, OUTPUT);

	pinMode(A4, INPUT);
	digitalWrite(A4, HIGH);
	delay(20);

	if (digitalRead(A4)) {
		isRX = true;
	}

	Serial.begin(115200);

	radio.begin();

	uint8_t addr[] = {0xaa, 0xbb, 0xcc, 0xdd, 0xee};
	if (isRX) {
		pinMode(SWITCH_PIN, OUTPUT);
		digitalWrite(SWITCH_PIN, HIGH);

		radio.set_rx_address(1, "slave");
		radio.start_listening();
	}

    // setting up environment
};

void loop() {
	if (isRX) {
		if (radio.available()) {
			uint8_t length = radio.get_payload_width();
			uint8_t buf[length];
			radio.get_payload(buf, length);

			Serial.print("Recived: ");
			if (length > 0) Serial.println(buf[0]);

			if (length > 0) {
				if (buf[0] == 42) digitalWrite(SWITCH_PIN, LOW);
				else digitalWrite(SWITCH_PIN, HIGH);;
			}
		}
	} else {
		if (Serial.available()) {
			char c = Serial.read();
			uint8_t val;
			if (c == 'i') {
				Serial.print("Put the light ON... ");
				val = 42;
			} else if (c == 'o') {
				Serial.print("Put the light OFF... ");
				val = 24;
			}

			Serial.println(radio.send_to("slave", &val, 1) ? " DONE" : " FAILED");
		}
	}
}
