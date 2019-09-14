#include "lcd.h"

#include <string.h>
#include <util/delay.h>

static char int_to_hex(char);

/* Initializes the lcd with the following configuration:
   * 4-bit mode
   * 8-dot font
   * No blinking
   * No cursor
   * Screen on
*/
void lcd_init(void) {
    /* Make sure all pins connected to the screen are set at output */
    LCD_DDR = 0xFF;
    LCD_PORT = 0x00;

    /* Initialization sequence from from p.46 in the HD44780 manual */

    /* Wait for more than 40ms */
    _delay_ms(80);

    /* Function set and wait more than 4.1ms */
    lcd_send_command(0, 0, 0x3);
    _delay_ms(10); 

    /* Function set again and wait more than 100us */
    lcd_send_command(0, 0, 0x3);
    _delay_ms(1);
    
    /* Last function set in 8-bit mode */
    lcd_send_command(0, 0, 0x3);

    /* Configuration sequence follows */
    lcd_send_command(0, 0, 0x2);

    /* Function set with 2 lines and 8 dot-font */
    lcd_send_command(0, 0, 0x2);
    lcd_send_command(0, 0, 0xC);

    /* Display off */
    lcd_send_command(0, 0, 0x0);
    lcd_send_command(0, 0, 0x8);

    /* Display clear */
    lcd_clear();

    /* Entry mode set with auto increment address and no shift */
    lcd_send_command(0, 0, 0x0);
    lcd_send_command(0, 0, 0x6);

    /* Switch display on with cursor and blink off */
    lcd_send_command(0, 0, 0x0);
    lcd_send_command(0, 0, 0xC);

    lcd_clear_and_home();
}

/*
  lcd_send_command sends a nibble of data to the screen.
  Implements p22 of the HITACHI HD44780 manual.
  All timings taken from p52.
 */
void lcd_send_command(int rs, int rw, char data) {
    LCD_PORT = (1<<LCD_PIN_EN) | (rs<<LCD_PIN_RS) | (rw<<LCD_PIN_RW) | (0x0F & data);

    /* Enable pulse width (high level) must be at least 230ns */
    _delay_us(1);

    /* Now bring ENABLE down for at lest 500ns and then back up */
    LCD_PORT &= ~(1<<LCD_PIN_EN);
    _delay_us(1);
    LCD_PORT |= (1<<LCD_PIN_EN);

    /* Wait for instruction to complete */
    _delay_us(100);
}

void lcd_print_char(char c) {
    lcd_send_command(1, 0, (c & 0xF0) >> 4);
    lcd_send_command(1, 0, c & 0x0F);
}

void lcd_clear(void) {
    lcd_send_command(0, 0, 0x0);
    lcd_send_command(0, 0, 0x1);
    _delay_ms(4);
}

void lcd_return_home(void) {
    lcd_send_command(0, 0, 0x0);
    lcd_send_command(0, 0, 0x2);
    _delay_ms(4);
}

void lcd_move_first_line(void) {
    lcd_send_command(0, 0, 0x8);
    lcd_send_command(0, 0, 0x0);
}

void lcd_move_second_line(void) {
    lcd_send_command(0, 0, 0xC);
    lcd_send_command(0, 0, 0x0);
}

/* print a null-terminating string on the screen */
void lcd_print_text(const char* t) {
    for(int i = 0; i < strlen(t); i++) {
	lcd_print_char(t[i]);
    }
}

/* print a string on the string */
void lcd_print_text2(const char* t, int len) {
    for(int i = 0; i < len; i++) {
	lcd_print_char(t[i]);
    }
}

/* clear the screen and print two null-terminating strings in different lines */
void lcd_show_lines(const char* line1, const char* line2) {
    lcd_clear_and_home();

    lcd_print_text(line1);
    lcd_move_second_line();
    lcd_print_text(line2);
}

/* like lcd_show_lines but the line separator is a '\n' character in the string */
void lcd_show_text(const char* t) {
    int second_line = 0;
    lcd_clear_and_home();

    for(int i = 0; i < 16; i++) {
	if (t[i] == '\0') {
	    return;
	}
	if (t[i] == '\n') {
	    lcd_move_second_line();
	    second_line = 1;
	    continue;
	}
	lcd_print_char(t[i]);
    }

    if (second_line == 0) {
	lcd_move_second_line();
    }

    for(int i = 0; i < 16; i++) {
	if (t[i+16] == '\0' || t[i+16] == '\n') {
	    return;
	}
	lcd_print_char(t[i+16]);
    }
}

void lcd_clear_and_home(void) {
    lcd_clear();
    lcd_return_home();
}

void lcd_print_byte(char b) {
    char high = (b & 0xF0) >> 4;
    char low = (b & 0x0F);
    lcd_print_text("0x");
    lcd_print_char(int_to_hex(high));
    lcd_print_char(int_to_hex(low));
}

static char int_to_hex(char c) {
    char r = 0;

    if (c > 9) {
	r = c + 55;
    } else {
	r = c + 48;
    }

    return r;
}

void lcd_display_on(void) {
    lcd_send_command(0, 0, 0x0);
    lcd_send_command(0, 0, 0xC);
    _delay_us(40);
}

void lcd_display_off(void) {
    lcd_send_command(0, 0, 0x0);
    lcd_send_command(0, 0, 0x8);
    _delay_us(40);
}
