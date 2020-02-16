#ifndef _LCD_H_
#define _LCD_H_

#ifndef F_CPU
#define F_CPU 8000000UL
#endif

#include <avr/io.h>

/* LCD port configuration */
#define LCD_PORT PORTD
#define LCD_DDR DDRD

#define LCD_PIN_DB4 PD0
#define LCD_PIN_DB5 PD1
#define LCD_PIN_DB6 PD2
#define LCD_PIN_DB7 PD3
#define LCD_PIN_RW PD4
#define LCD_PIN_RS PD5
#define LCD_PIN_EN PD6

void lcd_init(void);
void lcd_send_command(int, int, char);
void lcd_clear(void);
void lcd_return_home(void);
void lcd_move_first_line(void);
void lcd_move_second_line(void);
void lcd_print_char(char);
void lcd_print_byte(char);
void lcd_print_text(const char*);
void lcd_print_text2(const char*, int);
void lcd_show_lines(const char*, const char*);
void lcd_show_text(const char*);
void lcd_clear_and_home(void);

#endif
