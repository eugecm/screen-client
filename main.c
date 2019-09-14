#define F_CPU 8000000UL

#include "lcd.h"
#include "nrf24.h"

#include <avr/cpufunc.h>
#include <avr/io.h>
#include <util/delay.h>

char SERVER_ADDR[5] = {0xAB, 0xAB, 0xAB, 0xAB, 0xAB};
char* MSG = "HELLO";

void show_error(const char*);

void init(void) {
    lcd_init();
    nrf24_init(SERVER_ADDR);

    /* Set unused ports as input with pull-up resistors */
    DDRC = 0; PORTC = 0xFF;

    /* NOP used for synchronization */
    _NOP();
}

int main(void) {
    char buf;
    /* We make recv size 33 to append a \0 in case sender uses full 32 bytes*/
    char recv[33];
    char cfg;
    char status;

    init();

    /* Power up and standby-I */
    nrf24_send_command(NRF24_CMD_R_REGISTER | NRF24_REG_CONFIG, 0, 0, &cfg, 1);
    cfg = cfg | 0b00000010;
    nrf24_send_command(NRF24_CMD_W_REGISTER | NRF24_REG_CONFIG, &cfg, 1, 0, 0);
    _delay_ms(3);

    /* Begin transmission every 5 seconds */

    while (1) {
	lcd_clear_and_home();

	/* Prepare and write */
	nrf24_send_command(NRF24_CMD_W_REGISTER | NRF24_REG_TX_ADDR, SERVER_ADDR, 5, 0, 0);
	nrf24_send_command(NRF24_CMD_W_TX_PAYLOAD, MSG, 5, 0, 0);
	NRF24_SPI_PORT |= (1<<NRF24_PIN_CE);
	_delay_us(20); /* Hold for at least 10us to trigger TX */

	/* Wait for settling mode for at least 130us */
	_delay_us(200);

	/* Wait a bit longer for the transaction to go through */
	/* This should be higher than the worst-case scenario of auto-retries */
	_delay_ms(50);

	/* Back to standby-I and print status */
	NRF24_SPI_PORT &= ~(1<<NRF24_PIN_CE);

	_delay_ms(1);
	nrf24_send_command(NRF24_CMD_R_REGISTER | NRF24_REG_STATUS, 0, 0, &status, 1);

	/* If MAX_RT is asserted it means TX failed. Clear the flag */
	if (status & (1<<4)) {
	    status |= (1<<4); /* We clear by writing 1 */
	    nrf24_send_command(NRF24_CMD_W_REGISTER | NRF24_REG_STATUS, &status, 1, 0, 0);
	    show_error("TX FAILED");

	/* If TX_DS is up it means we've successfully sent the packet */
	} else if (status & (1<<5)) {

	    status |= (1<<5); /* We clear by writing 1 */
	    nrf24_send_command(NRF24_CMD_W_REGISTER | NRF24_REG_STATUS, &status, 1, 0, 0);

	    /* Check that we've received an ACK payload */
	    if (status & (1<<6)) {
		status |= (1<<6); /* We clear by writing 1 */
		nrf24_send_command(NRF24_CMD_W_REGISTER | NRF24_REG_STATUS, &status, 1, 0, 0);

		/* Get the length of the ack payload. */
		nrf24_send_command(NRF24_CMD_R_RX_PL_WID, 0, 0, &buf, 1);

		/* Read the payload and show it*/
		nrf24_send_command(NRF24_CMD_R_RX_PAYLOAD, 0, 0, recv, buf);

		recv[buf] = '\0';
		lcd_show_text(recv);
	    } else {
		show_error("NO PAYLOAD");
	    }
	} else {
	    show_error("MSG NOT SENT");
	}

	/* Wait before next transmission */
	_delay_ms(1000);
	nrf24_send_command(NRF24_CMD_FLUSH_TX, 0, 0, 0, 0);
    }

    return 1;
}

void show_error(const char* err) {
    lcd_show_lines("[ERROR]", err);
}
