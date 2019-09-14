#include "nrf24.h"

#include <util/delay.h>


/* Initializes the SPI port and the NRF24 chip */
void nrf24_init(char* server_addr) {
    /* Configure SPI for communicating with the NRF24 */
    {
	/* Configure SPI PINs as output */
	NRF24_SPI_DDR = (1<<NRF24_PIN_SCK) |
	    (1<<NRF24_PIN_MOSI) | (1<<NRF24_PIN_SS) | (1<<NRF24_PIN_CE);

	/* Configure SPI as Master and fck/16 */
	SPCR = (1<<SPE) | (1<<MSTR) | (1<<SPR0);
    }

    /* Pull CSN high. All commands will cycle from high to low */
    NRF24_SPI_PORT |= (1<<NRF24_PIN_SS);

    /* Configure chip */

    /* Wait at least 100ms for the chip to be in "Power Down" mode. (p.21 of Manual) */
    _delay_ms(110);

    /* We can configure the chip in Power Down mode. */

    /* General configuration parameters (p.54) :
       - cfg[7] = 0; Reserved.
       - cfg[6] = 1; Disable interrupt on RX (we're not going to use it).
       - cfg[5] = 1; Disable interrupt on TX (we're not going to use it).
       - cfg[4] = 1; Disable interrupt on MAX_RT (we're not going to use it).
       - cfg[3] = 1; Enable CRC.
       - cfg[2] = 0; Use 1-byte CRC.
       - cfg[1] = 0; Power down. We stay in this mode while we configure more params.
       - cfg[0] = 0; Configure as PTX. We'll be sending packets to the server.
    */
    char cfg = 0b01111000;
    nrf24_send_command(NRF24_CMD_W_REGISTER | NRF24_REG_CONFIG, &cfg, 1, 0, 0);

    /* We will receive data from the server in the form of an ACK. Enable
       auto ack on data pipe 0 */
    cfg = 0x3F;
    nrf24_send_command(NRF24_CMD_W_REGISTER | NRF24_REG_EN_AA, &cfg, 1, 0, 0);

    /* Enable data pipe 0 cfg[0]=1 for RX */
    cfg = 0b00000001;
    nrf24_send_command(NRF24_CMD_W_REGISTER | NRF24_REG_EN_RXADDR, &cfg, 1, 0, 0);

    /* Set up address widths. The chips will all have 5 cfg[1:0]=11 byte addresses */
    cfg = 0b00000011;
    nrf24_send_command(NRF24_CMD_W_REGISTER | NRF24_REG_SETUP_AW, &cfg, 1, 0, 0);

    /* In order to have a fair level or reliability, we set 2 auto retries (ARC=2)
       with a delay of 500us (ARD=500us). Note that the data rate is 1Mbps, which
       means it takes _at least_ 256us to receive a 32 byte payload. */
    cfg = 0b00010010; /* 0001 = 500us; 0010 = 2 */
    nrf24_send_command(NRF24_CMD_W_REGISTER | NRF24_REG_SETUP_RETR, &cfg, 1, 0, 0);

    /* Antenna parameters:
       cfg[7] = 0; Disable continuous carrier transmit.
       cfg[6] = 0; Reserved.
       cfg[5] = 0; Do not set data rate to 250kbps.
       cfg[4] = 0; Reserved.
       cfg[3] = 0; Enable high data rate to 1Mbps.
       cfg[2:1] = 11; Output power = 0dBm. (Might have to tweak this).
       cfg[0] = 0; Reserved.
    */
    cfg = 0b00000110;
    nrf24_send_command(NRF24_CMD_W_REGISTER | NRF24_REG_RF_SETUP, &cfg, 1, 0, 0);

    /* Set channel frequency to 2466 (2400 + 66) */
    cfg = 66;
    nrf24_send_command(NRF24_CMD_W_REGISTER | NRF24_REG_RF_CH, &cfg, 1, 0, 0);

    /* Set the RX address to match the server address on pipe 0
       so we can read the ack payload from the server after we've
       sent a message.
    */
    nrf24_send_command(NRF24_CMD_W_REGISTER | NRF24_REG_RX_ADDR_P0, server_addr, 5, 0, 0);

    /* Extra features:
       cfg[2] = 1; Enable dynamic payload length.
       cfg[1] = 1; Enable payloads with acknowledgements.
       cfg[0] = 0; Disable the W_TX_PAYLOAD_NOACK command.
    */
    cfg = 0b00000110;
    nrf24_send_command(NRF24_CMD_W_REGISTER | NRF24_REG_FEATURE, &cfg, 1, 0, 0);

    /* Dynamic payload length on data pipe 0 */
    cfg = 0b00000001;
    nrf24_send_command(NRF24_CMD_W_REGISTER | NRF24_REG_DYNPD, &cfg, 1, 0, 0);
}

/* Sends a command to the NRF24 via SPI. p47 of the manual.
   Can also read an output if in_len > 0. */
void nrf24_send_command(char cmd, char* out, int out_len, char* in, int in_len) {
    /* Commands always begin with a high-to-low SS signal
       so we pull SS low. (Assume we come from high?) */
    NRF24_SPI_PORT &= ~(1<<NRF24_PIN_SS);
    _delay_us(1);

    /* Write command word */
    SPDR = cmd;
    while(!(SPSR & (1<<SPIF)));

    /* Write command data */
    for(int i = 0; i < out_len; i++) {
	SPDR = out[i];
	while(!(SPSR & (1<<SPIF)));
    }

    /* Read output from chip (if in_len > 0) */
    for(int i = 0; i < in_len; i++) {
	/* Write a NOP to read a byte */
	SPDR = NRF24_CMD_NOP;
	while(!(SPSR & (1<<SPIF)));
	in[i] = SPDR;
    }

    /* Pull CSN high again */
    NRF24_SPI_PORT |= (1<<NRF24_PIN_SS);
    _delay_us(1);
}
