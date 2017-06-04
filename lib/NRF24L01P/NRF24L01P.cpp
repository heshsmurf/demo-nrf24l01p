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
    while (!(get_status() & (_BV(MAX_RT) | _BV(TX_DS)))) {}
    digitalWrite(ce_pin, LOW);
}

uint8_t Radio::read_register(uint8_t reg) {
    start_transaction();
    SPI.transfer(COMMANDS::R_REGISTER | reg);
    uint8_t result = SPI.transfer(COMMANDS::NOP);
    end_transaction();

    return result;
}

uint8_t Radio::read_register(uint8_t reg, uint8_t *buffer, uint8_t length) {
	start_transaction();
	uint8_t status = SPI.transfer(COMMANDS::R_REGISTER | reg);
	while (length--) *buffer++ = SPI.transfer(COMMANDS::NOP);
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

void Radio::send_to(uint8_t* address, uint8_t* payload, uint8_t length) {
	set_tx_address(address);
	set_rx_address(0, address);

	clear_tx();

	start_transaction();
	SPI.transfer(COMMANDS::W_TX_PAYLOAD);
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
    // write_register(REGISTER_MAP::STATUS, _BV(MAX_RT) | _BV(RX_DR) | _BV(TX_DS));

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
    // uint8_t status = get_status();
	uint8_t status = read_register(REGISTER_MAP::FIFO_STATUS);
    return !(status & 1);
}

uint8_t Radio::get_payload_width() {
    start_transaction();
    SPI.transfer(COMMANDS::R_RX_PL_WID);
    uint8_t result = SPI.transfer(COMMANDS::NOP);
    end_transaction();

    return result;
}

uint8_t Radio::get_payload(uint8_t* buffer, uint8_t length) {
	write_register(REGISTER_MAP::STATUS, _BV(RX_DR));

	start_transaction();
	uint8_t status = SPI.transfer(COMMANDS::R_RX_PAYLOAD);
	// while (length--) *buffer++ = SPI.transfer(COMMANDS::NOP);
	for (int i = 0; i < length; i++)
		buffer[i] = SPI.transfer(COMMANDS::NOP);
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

	// write_register(REGISTER_MAP::FEATURE, _BV(EN_DPL) | _BV(EN_DYN_ACK));
	write_register(REGISTER_MAP::FEATURE, 0);
	// TODO: add bit mnemonics for DYNPD
	write_register(REGISTER_MAP::DYNPD, 0);

	// write_register(REGISTER_MAP::SETUP_RETR, 0b00110011);

	write_register(REGISTER_MAP::STATUS, _BV(MAX_RT) | _BV(RX_DR) | _BV(TX_DS));

	write_register(REGISTER_MAP::RX_PW_P0, 4);
	write_register(REGISTER_MAP::RX_PW_P1, 4);

	address_width = (0x03 & read_register(REGISTER_MAP::SETUP_AW)) + 2;

	flush_rx();
	flush_tx();
}

void Radio::get_rx_address(uint8_t pipe, uint8_t* buffer) {
	read_register(REGISTER_MAP::RX_ADDR_P0 + pipe, buffer, address_width);
}

void Radio::set_rx_address(uint8_t pipe, uint8_t* buffer) {
	write_register(REGISTER_MAP::RX_ADDR_P0 + pipe, buffer, address_width);
}

void Radio::set_tx_address(uint8_t* buffer) {
	write_register(REGISTER_MAP::TX_ADDR, buffer, address_width);
}
