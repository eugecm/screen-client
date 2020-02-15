#define F_CPU 8000000UL

#include <alloca.h>
#include <string.h>
#include <avr/cpufunc.h>
#include <avr/io.h>
#include <util/delay.h>

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
#define NRF24_REG_RX_PW_P0 0x11
#define NRF24_REG_DYNPD 0x1C
#define NRF24_REG_FEATURE 0x1D

void init(void);

char nrf24_read_register(char);
void nrf24_write_register(char, char);
void nrf24_write_register_n(char, char*, int);
char nrf24_read_payload_width(void);
void nrf24_read_rx_payload(char*, int);

char int_to_hex(char);

char* RX_ADDRESS = "ABABA";

int main(void)
{
    char data;

    init();
    lcd_init();

    lcd_clear_and_home();

    // read STATUS
    data = nrf24_read_register(0x07);
    lcd_print_text("STATUS=");
    lcd_print_byte(data);

    lcd_move_second_line();

    // read CONFIG
    data = nrf24_read_register(0x00);
    lcd_print_text("CONFIG=");
    lcd_print_byte(data);

    // Making sure CE is low
    PORTB &= ~(1<<PB1);
    _delay_ms(1000);

    // Configure RX mode
    {
	// Make active RX (PRIM_RX=1), with CRC 1 byte
	nrf24_write_register(NRF24_REG_CONFIG, 0b00001011);

	// POWER DOWN to STANDBY takes 1.5ms
	_delay_ms(3);

	// Enable data pipe 0
	nrf24_write_register(NRF24_REG_EN_RXADDR, 0x01);

	// Disable auto acknowledgement on all pipes (for compatibility with NRF24)
	nrf24_write_register(NRF24_REG_EN_AA, 0x00);

	// Disable retransmissions
	nrf24_write_register(NRF24_REG_SETUP_RETR, 0x00);

	// Set 1mbps, -18dBm
	nrf24_write_register(NRF24_REG_RF_SETUP, 0b00000000);

	// Set 2416MHz
	nrf24_write_register(NRF24_REG_RF_CH, 0x10);

	// Disable features
	nrf24_write_register(NRF24_REG_FEATURE, 0x00);

	// Disable dynamic payload for everything
	nrf24_write_register(NRF24_REG_DYNPD, 0x00);

	// Set address length of 5 bytes
	nrf24_write_register(NRF24_REG_SETUP_AW, 0x03);

	// Set address
	nrf24_write_register_n(NRF24_REG_RX_ADDR_P0, RX_ADDRESS, 5);

	// Set payload width (32 bytes) (for compatibility with NRF24)
	nrf24_write_register(NRF24_REG_RX_PW_P0, 0b00100000);

	_delay_us(500);
	PORTB |= (1<<PB1);
	_delay_us(500);
    }

    lcd_show2("WAITING...");

    // Loop for a packet and display it
    // Receive loop:
    char status;
    char buf[32];
    while (1) {
	// poll RX_DR from STATUS (if high means data received)
	status = nrf24_read_register(NRF24_REG_STATUS);

	if ((status & 0x40) == 0) {
	    _delay_ms(1000);
	    continue;
	}

	// Wait and make CE low (to go to standby I)
	PORTB &= ~(1<<PB1);

	// Read the payload and display it
	nrf24_read_rx_payload(buf, 32);
	lcd_show2(buf);

	// clear the status
	nrf24_write_register(NRF24_REG_STATUS, (status | 0x20));

	// Go back to RX mode
	PORTB |= (1<<PB1);
	_delay_us(500);
    }

    return 1;
}

void nrf24_write_register(char addr, char data)
{
    // Pull CSN low
    PORTB &= ~(1<<PB2);
    _delay_us(1);

    // Write command
    SPDR = 0x20 | addr;
    while(!(SPSR & (1<<SPIF)));

    // Send data
    SPDR = data;
    while(!(SPSR & (1<<SPIF)));

    // Pull CSN high again
    PORTB |= (1<<PB2);
    _delay_us(1);
}

void nrf24_write_register_n(char addr, char* data, int len)
{
    // Pull CSN low
    PORTB &= ~(1<<PB2);
    _delay_us(1);

    // Write command
    SPDR = 0x20 | addr;
    while(!(SPSR & (1<<SPIF)));

    // Send data
    for(int i = 0; i < len; i++)
    {
	SPDR = data[i];
	while(!(SPSR & (1<<SPIF)));
    }

    // Pull CSN high again
    PORTB |= (1<<PB2);
    _delay_us(1);
}

void nrf24_read_rx_payload(char* buf, int len)
{
    // Pull CSN low
    PORTB &= ~(1<<PB2);
    _delay_us(1);

    // Write R_RX_PAYLOAD
    SPDR = 0x61;
    while(!(SPSR & (1<<SPIF)));

    for(int i = 0; i<len; i++)
    {
	// Read byte
	SPDR = 0xFF;
	while(!(SPSR & (1<<SPIF)));

	buf[i] = SPDR;
    }

    // Pull CSN high again
    PORTB |= (1<<PB2);
    _delay_us(1);
}

char nrf24_read_payload_width(void)
{
    char data = 0;

    // Pull CSN high and wait a bit
    PORTB |= (1<<PB2);
    _delay_us(500);

    PORTB &= ~(1<<PB2);
    _delay_us(1);

    // Write R_RX_PL_WID command (0110 0000)
    SPDR = 0x60;
    while(!(SPSR & (1<<SPIF)));

    // Write a NOP to read the contents
    SPDR = 0xFF;
    while(!(SPSR & (1<<SPIF)));
    PORTB |= (1<<PB2);
    _delay_us(1);

    data = SPDR;
    return data;
}
char nrf24_read_register(char addr)
{
    char data = 0;

    // Pull CSN high and wait a bit
    PORTB |= (1<<PB2);
    _delay_us(500);

    PORTB &= ~(1<<PB2);
    _delay_us(1);

    // Write read command (000A AAAA)
    SPDR = 0x00 | addr;
    while(!(SPSR & (1<<SPIF)));

    // Write a NOP to read the contents
    SPDR = 0xFF;
    while(!(SPSR & (1<<SPIF)));
    PORTB |= (1<<PB2);
    _delay_us(1);

    data = SPDR;
    return data;
}

void init(void)
{
    /* Set all port D as output (lcd port) */
    DDRD = 0xFF;
    PORTD = 0x00;

    // Configure PORTB for SPI (master)
    // MOSI is PB3 and SCK is PB5, ^SS is PB2. CE is PB1
    DDRB = (1<<PB5) | (1<<PB3) | (1<<PB2) | (1<<PB1);

    // Configure SPI as Master and fck/16
    SPCR = (1<<SPE) | (1<<MSTR) | (1<<SPR0);

    /* Set all other ports as input with pull-up resistors */
    DDRC = 0; PORTC = 0xFF;

    /* NOP used for synchronization */
    _NOP();
}
