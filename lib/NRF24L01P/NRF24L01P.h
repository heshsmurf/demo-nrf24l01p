#ifndef _NRF24L01P_H_
#define _NRF24L01P_H_

#include "Arduino.h"
#include "SPI.h"

enum COMMANDS {
    R_REGISTER          = 0x00,
    W_REGISTER          = 0x20,
    R_RX_PAYLOAD        = 0x61,
    W_TX_PAYLOAD        = 0xA0,
    FLUSH_TX            = 0xE1,
    FLUSH_RX            = 0xE2,
    REUSE_TX_PL         = 0xE3,
    R_RX_PL_WID         = 0x60,
    W_TX_PAYLOAD_NO_ACK = 0xB0,
    NOP                 = 0xFF,
};

enum REGISTER_MAP {
    CONFIG      = 0x00,
    EN_AA       = 0x01,
    EN_RXADDR   = 0x02,
    SETUP_AW    = 0x03,
    SETUP_RETR  = 0x04,
    RF_CH       = 0x05,
    RF_SETUP    = 0x06,
    STATUS      = 0x07,
    OBSERVE_TX  = 0x08,
    RPD         = 0x09,
    RX_ADDR_P0  = 0x0A,
    RX_ADDR_P1  = 0x0B,
    RX_ADDR_P2  = 0x0C,
    RX_ADDR_P3  = 0x0D,
    RX_ADDR_P4  = 0x0E,
    RX_ADDR_P5  = 0x0F,
    TX_ADDR     = 0x10,
    RX_PW_P0    = 0x11,
    RX_PW_P1    = 0x12,
    RX_PW_P2    = 0x13,
    RX_PW_P3    = 0x14,
    RX_PW_P4    = 0x15,
    RX_PW_P5    = 0x16,
    FIFO_STATUS = 0x17,
    DYNPD       = 0x1C,
    FEATURE     = 0x1D
};

enum BIT_MNEMONIC {
    // CONFIG
    MASK_RX_DR  = 6,
    MASK_TX_DS  = 5,
    MASK_MAX_RT = 4,
    EN_CRC      = 3,
    CRCO        = 2,
    PWR_UP      = 1,
    PRIM_RX     = 0,

    // EN_RXADDR
    ERX_P5 = 5,
    ERX_P4 = 4,
    ERX_P3 = 3,
    ERX_P2 = 2,
    ERX_P1 = 1,
    ERX_P0 = 0,

    // SETUP_RETR
    ARD = 4,
    ARC = 0,

    // STATUS
    RX_DR   = 6,
    TX_DS   = 5,
    MAX_RT  = 4,
    TX_FULL = 0,

    // FEATURE
    EN_DPL     = 2,
    EN_ACK_PAY = 1,
    EN_DYN_ACK = 0
};

class Radio {
private:
    uint8_t ss_pin;
    uint8_t ce_pin;
    uint8_t address_width;
	uint8_t sid;

    void send_packet();


public:
    /**
    */
    void start_transaction();

    /**
    */
    void end_transaction();

    /**
    */
    uint8_t read_register(uint8_t reg);

	uint8_t read_register(uint8_t reg, void* buffer, uint8_t length);

    uint8_t write_register(uint8_t reg, uint8_t value);

    uint8_t write_register(uint8_t reg, void* value, uint8_t length);

    uint8_t execute_cmd(uint8_t cmd);

    void send_no_ack(uint8_t* payload, uint8_t length);

    void start_listening();

	void stop_listening();

    uint8_t get_status();

    bool available();

    uint8_t get_payload(void* buffer, uint8_t length);

	uint8_t get_payload_width();

	void begin();

	bool send_to(void* address, void* payload, uint8_t length);

	void clear_tx();

	uint8_t get_config();

	void flush_tx();

	void flush_rx();

	void set_tx_address(void* address);

	void get_tx_address(void* address);

	void set_rx_address(uint8_t pipe, void* address);

	void get_rx_address(uint8_t pipe, void* buffer);

    Radio(uint8_t _ss_pin, uint8_t _ce_pin);
};

#endif
