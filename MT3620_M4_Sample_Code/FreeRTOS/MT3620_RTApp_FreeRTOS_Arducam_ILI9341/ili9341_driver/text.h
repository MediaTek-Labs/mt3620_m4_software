#ifndef TEXT_H
#define TEXT_H

#include <stdint.h>

void lcd_display_char(char c);
void lcd_display_string(char *p_str);
void lcd_set_text_cursor(uint16_t x, uint16_t y);
void lcd_set_text_size(uint8_t size);
void lcd_set_text_color(uint16_t color);
void lcd_set_text_bg_color(uint16_t color);

#endif
