#include <stdint.h>

#include "ili9341.h"
#include "ili9341_ll.h"
#include "font.h"
#include "text.h"


#define FEATURE_CP437		0
#define FEATURE_WRAP		1

uint16_t s_cursorX, s_cursorY;
uint8_t  s_textSize = 1;

uint16_t s_textColor = BLACK;
uint16_t s_textBgColor = WHITE;

static void _draw_char(uint16_t x0, uint16_t y0, unsigned char c, uint16_t color, uint16_t bg, uint8_t size) 
{
	uint16_t scaledWidth = (uint16_t)(size * 6),
		doubleScaledWidth = scaledWidth * size;

	uint16_t x1 = (uint16_t)(x0 + scaledWidth - 1),
		y1 = (uint16_t)(y0 + 8 * size - 1);

	uint16_t doubleSize = size * size;
	uint16_t count = (uint16_t)(48 * doubleSize);

	uint16_t charPixels[count];

	uint16_t mx, my;
	int8_t i, j, sx, sy;
	uint8_t  line;
	uint16_t pixelColor;

	if (x0 >= 320 || y0 >= 240 || x1 < 0 || y1 < 0) return;

#if (defined(FEATURE_CP437) && (FEATURE_CP437 == 0))
	if (c >= 176) c++; // Handle 'classic' charset behavior
#endif

	uint16_t characterNumber = (uint16_t)(c * 5);

	ili9341_set_window(x0, y0, x1, y1);

	for (i = 0; i < 6; i++) {
		line = (uint8_t)(i < 5 ? *(font + characterNumber + i) : 0x0);
		my = (uint16_t)(i * size);


		for (j = 0; j < 8; j++, line >>= 1) {
			mx = (uint16_t)(j * doubleScaledWidth);

			pixelColor = line & 0x1 ? color : bg;

			for (sx = 0; sx < size; ++sx) {
				for (sy = 0; sy < size; ++sy) {
					charPixels[mx + my + sy * scaledWidth + sx] = pixelColor;
				}
			}
		}
	}

	ili9341_ll_dc_high();
	ili9341_ll_spi_tx((uint8_t*)charPixels, count * 2);
}

void lcd_display_char(char c) 
{	
	if (c == '\n') {

		s_cursorY += s_textSize * 8;
		s_cursorX = 0;

	} else if (c == '\r') {

		s_cursorX = 0;

	} else {

#if (defined(FEATURE_WRAP) && (FEATURE_WRAP == 1))
		if (((s_cursorX + s_textSize * 6) >= 320)) {
			s_cursorX = 0;
			s_cursorY += s_textSize * 8;
		}
#endif

		_draw_char(s_cursorX, s_cursorY, c, s_textColor, s_textBgColor, s_textSize);
		s_cursorX += s_textSize * 6;
	}
}

void lcd_display_string(char *p_str) 
{
	while (*(p_str) != '\0') {
		lcd_display_char(*p_str++);
	}
}

void lcd_set_text_cursor(uint16_t x, uint16_t y)
{
	s_cursorX = x;
	s_cursorY = y;
}

void lcd_set_text_size(uint8_t size) 
{
	s_textSize = size;
}

void lcd_set_text_color(uint16_t color)
{
	s_textColor = color;
}

void lcd_set_text_bg_color(uint16_t color) 
{
	s_textBgColor = color;
}
