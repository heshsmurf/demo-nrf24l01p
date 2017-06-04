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
	pinMode(SWITCH_PIN, OUTPUT);
	digitalWrite(SWITCH_PIN, LOW);

	Serial.begin(115200);

	radio.begin();

	Serial.print("I'm a ");
	Serial.println(isRX ? "receiver" : "sender");

	currentState = STATE::BINDING;
	if (isRX) {
		radio.set_rx_address(1, "node1");
		radio.start_listening();
	}

    // setting up environment
};

// void loop() {
//     switch (currentState) {
//         case STATE::BINDING: {
//             // radio.ping_master();
//             radio.start_listening();
//             delay(5);
//             if (radio.available()) {
// 				uint8_t length = radio.get_payload_width();
// 				uint8_t buf[length];
// 				radio.get_payload(buf, length);
//
// 				if (length > 0 && buf[0] == 1 && buf[0] == 4 && buf[0] == 8 && buf[0] == 8)
// 					digitalWrite(LED_BUILTIN, HIGH);
//             }
//             break;
//         }
//         case STATE::RUNNING: {
//             break;
//         }
//         default: break;
//     }
// };

void loop() {
	if (isRX) {
		if (radio.available()) {
			uint8_t length = radio.get_payload_width();
			Serial.print("Received ");
			Serial.print(length);
			Serial.print(" bytes: ");

			uint8_t buf[length];
			radio.get_payload(buf, length);
			for (int i = 0; i < length; i++) Serial.print(buf[i]);
			Serial.print("\n");
			// for (int i = 0; i < 4; i++) Serial.print((char)buf[i]);
			// Serial.println();

			if (length > 0 && buf[0] == 1 && buf[1] == 4 && buf[2] == 8 && buf[3] == 8)
				state = !state;
		}
	} else {
		if (Serial.available()) {
			char c = Serial.read();
			Serial.print("Get ");
			Serial.println(c);
			if (c == 't') {
				Serial.println("Now send command");
				state = !state;
				uint8_t buf[] = {1, 4, 8, 8};
				radio.send_to("node1", buf, 4);
				// while ( (status = radio.get_status()) & _BV(TX_DS) == 0 ) {
				// 	Serial.println(status, BIN);
				// 	delay(300);
				// }
				Serial.println(radio.get_status(), BIN);
			}
		}
	}
	// Serial.print("STATUS: ");
	// Serial.println(radio.get_status(), BIN);

	// if (Serial.available()) {
	// 	char c = Serial.read();
		// Serial.print("Entered: ");
		// Serial.print(c);
		// Serial.print("\n");
		// if (c == '1') {
			// Serial.print("SCHA vkluchim\n");

			// uint8_t buf[] = {1, 4, 8, 8};
			// radio.clear_tx();
			// radio.send(buf, 4);

			// Serial.print("status: ");
			// Serial.println(radio.get_status(), BIN);
			// Serial.print("\n");
	// 	}
	// }
	// state = !state;
	// Serial.print(state ? "HIGH\n" : "LOW\n");
	// Serial.println();
	if (state)
		digitalWrite(SWITCH_PIN, HIGH);
	else
		digitalWrite(SWITCH_PIN, LOW);
	// delay(1000);
}
