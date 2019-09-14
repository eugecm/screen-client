#ifndef _NRF24_H
#define _NRF24_H

#ifndef F_CPU
#define F_CPU 8000000UL
#endif

#include <avr/io.h>

/* Configuration */
#define NRF24_SPI_PORT PORTB
#define NRF24_SPI_DDR DDRB
#define NRF24_PIN_SCK PB5
#define NRF24_PIN_MOSI PB3
#define NRF24_PIN_SS PB2
#define NRF24_PIN_CE PB1

/* NRF24L01 registers */
#define NRF24_REG_CONFIG 0x00
#define NRF24_REG_EN_AA 0x01
#define NRF24_REG_EN_RXADDR 0x02
#define NRF24_REG_SETUP_AW 0x03
#define NRF24_REG_SETUP_RETR 0x04
#define NRF24_REG_RF_CH 0x05
#define NRF24_REG_RF_SETUP 0x06
#define NRF24_REG_STATUS 0x07
#define NRF24_REG_OBSERVE_TX 0x08
#define NRF24_REG_RX_ADDR_P0 0x0A
#define NRF24_REG_TX_ADDR 0x10
#define NRF24_REG_RX_PW_P0 0x11
#define NRF24_REG_DYNPD 0x1C
#define NRF24_REG_FEATURE 0x1D

/* NRF24L01 commands */
#define NRF24_CMD_R_REGISTER 0b00000000 /* OR with register address */
#define NRF24_CMD_W_REGISTER 0b00100000 /* OR with register address */
#define NRF24_CMD_R_RX_PAYLOAD 0b01100001
#define NRF24_CMD_W_TX_PAYLOAD 0b10100000
#define NRF24_CMD_FLUSH_TX 0b11100001
#define NRF24_CMD_FLUSH_RX 0b11100010
#define NRF24_CMD_R_RX_PL_WID 0b01100000
#define NRF24_CMD_NOP 0b11111111

void nrf24_init(char*);
void nrf24_send_command(char, char*, int, char*, int);

#endif
