#include "NRF24L01P.h"

Radio::Radio(uint8_t _ss_pin, uint8_t _ce_pin): ss_pin(_ss_pin), ce_pin(_ce_pin) {
    // enable dyn payload for all pipes
};

void Radio::start_transaction() {
    digitalWrite(ss_pin, LOW);
}

void Radio::end_transaction() {
    digitalWrite(ss_pin, HIGH);
}

void Radio::send_packet() {
    digitalWrite(ce_pin, HIGH);
    delayMicroseconds(15);
    digitalWrite(ce_pin, LOW);
}

uint8_t Radio::read_register(uint8_t reg) {
    start_transaction();
    SPI.transfer(COMMANDS::R_REGISTER | reg);
    uint8_t result = SPI.transfer(COMMANDS::NOP);
    end_transaction();

    return result;
}

uint8_t Radio::write_register(uint8_t reg, uint8_t value) {
    start_transaction();
    uint8_t status = SPI.transfer(COMMANDS::W_REGISTER | reg);
    SPI.transfer(value);
    end_transaction();

    return status;
}

uint8_t Radio::write_register(uint8_t reg, uint8_t* value, uint8_t length) {
    start_transaction();
    uint8_t status = SPI.transfer(COMMANDS::W_REGISTER | reg);
    while (length--) SPI.transfer(*value++);
    end_transaction();

    return status;
}

void Radio::send_no_ack(uint8_t* payload, uint8_t length) {
    length = length > 32 ? 32 : length;
    start_transaction();
    SPI.transfer(COMMANDS::W_TX_PAYLOAD_NO_ACK);
    while (length--) SPI.transfer(*payload++);
    end_transaction();

    send_packet();
}

uint8_t Radio::execute_cmd(uint8_t cmd) {
    start_transaction();
    uint8_t result = SPI.transfer(cmd);
    end_transaction();
    return result;
}

void Radio::ping_master() {
    uint8_t address[] = {0, 0, 0, 0, 0};
    write_register(REGISTER_MAP::TX_ADDR, address, 5);

    // send_no_ack(uint[], 5);

}

void Radio::start_listening() {
    write_register(REGISTER_MAP::CONFIG, read_register(REGISTER_MAP::CONFIG) | _BV(PWR_UP) | _BV(PRIM_RX));
    write_register(REGISTER_MAP::STATUS, _BV(MAX_RT) | _BV(RX_DR) | _BV(TX_DS));

    digitalWrite(ce_pin, HIGH);
    delayMicroseconds(150);
}

uint8_t Radio::get_status() {
    start_transaction();
    uint8_t status = SPI.transfer(COMMANDS::NOP);
    end_transaction();

    return status;
}

bool Radio::available() {
    uint8_t status = get_status();
    return status & _BV(RX_DR);
}

uint8_t Radio::get_payload_width() {
    start_transaction();
    SPI.transfer(COMMANDS::R_RX_PL_WID);
    uint8_t result = SPI.transfer(COMMANDS::NOP);
    end_transaction();

    return result;
}

uint8_t Radio::get_payload(uint8_t* buffer, uint8_t length) {
    
}
