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
    // do {
		delayMicroseconds(20);
	// } whitle (!(get_status() & (_BV(MAX_RT) | _BV(TX_DS))));

	digitalWrite(ce_pin, LOW);
}

uint8_t Radio::read_register(uint8_t reg) {
    start_transaction();
    SPI.transfer(COMMANDS::R_REGISTER | reg);
    uint8_t result = SPI.transfer(COMMANDS::NOP);
    end_transaction();

    return result;
}

uint8_t Radio::read_register(uint8_t reg, void *buffer, uint8_t length) {
	start_transaction();
	uint8_t* tmp = reinterpret_cast<uint8_t*>(buffer);
	uint8_t status = SPI.transfer(COMMANDS::R_REGISTER | reg);
	while (length--) *tmp++ = SPI.transfer(COMMANDS::NOP);
	end_transaction();

	return status;
}

uint8_t Radio::write_register(uint8_t reg, uint8_t value) {
    start_transaction();
    uint8_t status = SPI.transfer(COMMANDS::W_REGISTER | reg);
    SPI.transfer(value);
    end_transaction();

    return status;
}

uint8_t Radio::write_register(uint8_t reg, void* value, uint8_t length) {
    start_transaction();
	uint8_t* tmp = reinterpret_cast<uint8_t*>(value);
    uint8_t status = SPI.transfer(COMMANDS::W_REGISTER | reg);
    while (length--) SPI.transfer(*tmp++);
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

bool Radio::send_to(void* address, void* payload, uint8_t length) {
	set_tx_address(address);
	set_rx_address(0, address);

	clear_tx();

	uint8_t* tmp = reinterpret_cast<uint8_t*>(payload);

	start_transaction();
	SPI.transfer(COMMANDS::W_TX_PAYLOAD);
	while (length--) SPI.transfer(*tmp++);
	// SPI.transfer(24);
	end_transaction();

    send_packet();

	return get_status() & _BV(TX_DS);
}

uint8_t Radio::execute_cmd(uint8_t cmd) {
    start_transaction();
    uint8_t result = SPI.transfer(cmd);
    end_transaction();
    return result;
}

void Radio::start_listening() {
    write_register(REGISTER_MAP::CONFIG, read_register(REGISTER_MAP::CONFIG) | _BV(PWR_UP) | _BV(PRIM_RX));
    write_register(REGISTER_MAP::STATUS, _BV(MAX_RT) | _BV(RX_DR) | _BV(TX_DS));

    digitalWrite(ce_pin, HIGH);
    delayMicroseconds(150);
}

void Radio::stop_listening() {
	write_register(REGISTER_MAP::CONFIG, read_register(REGISTER_MAP::CONFIG) & ~_BV(PRIM_RX));

	digitalWrite(ce_pin, LOW);
	delayMicroseconds(30);
}

uint8_t Radio::get_status() {
    start_transaction();
    uint8_t status = SPI.transfer(COMMANDS::NOP);
    end_transaction();

    return status;
}

bool Radio::available() {
    return get_status() & _BV(RX_DR);
}

uint8_t Radio::get_payload_width() {
    start_transaction();
    SPI.transfer(COMMANDS::R_RX_PL_WID);
    uint8_t result = SPI.transfer(COMMANDS::NOP);
    end_transaction();

    return result;
}

uint8_t Radio::get_payload(void* buffer, uint8_t length) {
	write_register(REGISTER_MAP::STATUS, _BV(RX_DR));

	uint8_t* tmp = reinterpret_cast<uint8_t*>(buffer);
	start_transaction();
	uint8_t status = SPI.transfer(COMMANDS::R_RX_PAYLOAD);
	while (length--) *tmp++ = SPI.transfer(COMMANDS::NOP);
	end_transaction();

	return status;
}

void Radio::clear_tx() {
	uint8_t status = get_status();
	if (status & _BV(MAX_RT)) {
		write_register(REGISTER_MAP::STATUS, _BV(MAX_RT));
	}
	if (status & _BV(TX_FULL)) {
		flush_tx();
	}
}

void Radio::flush_tx() {
	start_transaction();
	SPI.transfer(COMMANDS::FLUSH_TX);
	end_transaction();
}

void Radio::flush_rx() {
	start_transaction();
	SPI.transfer(COMMANDS::FLUSH_RX);
	end_transaction();
}

uint8_t Radio::get_config() {
	return read_register(REGISTER_MAP::CONFIG);
}

void Radio::begin() {
	SPI.setBitOrder(MSBFIRST);
	SPI.begin();

	end_transaction();

	write_register(REGISTER_MAP::CONFIG, (read_register(REGISTER_MAP::CONFIG) | _BV(PWR_UP)) & ~_BV(PRIM_RX));

	delayMicroseconds(150);

	write_register(REGISTER_MAP::FEATURE, _BV(EN_DPL));
	write_register(REGISTER_MAP::DYNPD, 3);
	write_register(REGISTER_MAP::EN_AA, 3);

	write_register(REGISTER_MAP::SETUP_RETR, 0b00110011);

	write_register(REGISTER_MAP::STATUS, _BV(MAX_RT) | _BV(RX_DR) | _BV(TX_DS));

	address_width = (0x03 & read_register(REGISTER_MAP::SETUP_AW)) + 2;

	flush_rx();
	flush_tx();
}

void Radio::get_rx_address(uint8_t pipe, void* buffer) {
	read_register(REGISTER_MAP::RX_ADDR_P0 + pipe, buffer, 5);
}

void Radio::set_rx_address(uint8_t pipe, void* buffer) {
	write_register(REGISTER_MAP::RX_ADDR_P0 + pipe, buffer, 5);
}

void Radio::set_tx_address(void* buffer) {
	write_register(REGISTER_MAP::TX_ADDR, buffer, 5);
}

void Radio::get_tx_address(void* buffer) {
	read_register(REGISTER_MAP::TX_ADDR, buffer, 5);
}
