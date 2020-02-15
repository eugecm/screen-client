#include "lcd.h"

/* init_lcd configures the lcd:
   * 4-bit mode
   * 8-dot font
   * No blinking
   * No cursor
   * screen on
*/
void lcd_init(void)
{
    /* Perform initialization (from p.46 in the HD44780 manual) */

    /* Wait for more than 40ms */
    _delay_ms(80);

    /* Function set and wait more than 4.1ms */
    lcd_send_command(0, 0, 0x3);
    _delay_ms(10); 

    /* Function set again */
    lcd_send_command(0, 0, 0x3);
    _delay_ms(10);
    
    /* Last function set in 8-bit mode */
    lcd_send_command(0, 0, 0x3);

    lcd_send_command(0, 0, 0x2);

    /* Function set with 2 lines and 8 dot-font */
    lcd_send_command(0, 0, 0x2);
    lcd_send_command(0, 0, 0xC);

    // Display off
    lcd_send_command(0, 0, 0x0);
    lcd_send_command(0, 0, 0x8);

    // Display clear
    lcd_clear();

    // Entry mode set with auto increment address and no shift
    lcd_send_command(0, 0, 0x0);
    lcd_send_command(0, 0, 0x6);

    // Switch display on with cursor and blink off
    lcd_send_command(0, 0, 0x0);
    lcd_send_command(0, 0, 0xC);
}

void lcd_send_command(int rs, int rw, char data)
{
    /*
      The LCD controller is connected to PORTD using the following configuration:
          PD0 -> DB4
          PD1 -> DB5
          PD2 -> DB6
          PD3 -> DB7
          PD4 -> RW
          PD5 -> RS
          PD6 -> EN
    */

    PORTD = (1<<PD6) | (rs<<PD5) | (rw<<PD4) | (0x0F & data);

    // Enable pulse width (high level) must be at least 230ns
    _delay_us(1);

    PORTD &= ~(1<<PD6); // This signals the LCD controller to read the pins
    _delay_us(1);

    // Enable cycle must be at least 500ns
    PORTD |= (1<<PD6);

    // Wait for instruction to complete
    _delay_us(100);
}

void lcd_print_char(char c)
{
    lcd_send_command(1, 0, (c & 0xF0) >> 4);
    lcd_send_command(1, 0, c & 0x0F);
}

void lcd_clear(void)
{
    lcd_send_command(0, 0, 0x0);
    lcd_send_command(0, 0, 0x1);
    _delay_ms(4);
}

void lcd_return_home(void)
{
    lcd_send_command(0, 0, 0x0);
    lcd_send_command(0, 0, 0x2);
    _delay_ms(4);
}

void lcd_move_first_line(void)
{
    lcd_send_command(0, 0, 0x8);
    lcd_send_command(0, 0, 0x0);
}

void lcd_move_second_line(void)
{
    lcd_send_command(0, 0, 0xC);
    lcd_send_command(0, 0, 0x0);
}

void lcd_print_text(const char* t)
{
    for(int i = 0; i < strlen(t); i++)
    {
	lcd_print_char(t[i]);
    }
}

void lcd_print_text2(const char* t, int len)
{
    for(int i = 0; i < len; i++)
    {
	lcd_print_char(t[i]);
    }
}

void lcd_show(const char* line1, const char* line2)
{
    lcd_clear();
    lcd_return_home();

    lcd_print_text(line1);
    lcd_move_second_line();
    lcd_print_text(line2);
}

void lcd_show2(const char* t)
{
    int second_line = 0;
    lcd_clear();
    lcd_return_home();

    for(int i = 0; i < 16; i++)
    {
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

    for(int i = 0; i < 16; i++)
    {
	if (t[i+16] == '\0' || t[i+16] == '\n') {
	    return;
	}
	lcd_print_char(t[i+16]);
    }
}

void lcd_clear_and_home(void)
{
    lcd_clear();
    lcd_return_home();
}

void lcd_print_byte(char b)
{
    char high = (b & 0xF0) >> 4;
    char low = (b & 0x0F);
    lcd_print_text("0x");
    lcd_print_char(int_to_hex(high));
    lcd_print_char(int_to_hex(low));
}

char int_to_hex(char c)
{
    char r = 0;

    if (c > 9) {
	r = c + 55;
    } else {
	r = c + 48;
    }

    return r;
}
