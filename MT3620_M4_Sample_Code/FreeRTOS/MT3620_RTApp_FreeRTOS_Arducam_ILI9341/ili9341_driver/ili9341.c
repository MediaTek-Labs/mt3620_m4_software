#include "ili9341_ll.h"
#include "ili9341.h"

#if defined(AzureSphere_CA7)
#include "delay.h"
#elif defined(AzureSphere_CM4)
#include "FreeRTOS.h"
#include "task.h"
#endif

void ili9341_write_cmd(uint8_t reg)
{
	ili9341_ll_dc_low();
	(void)ili9341_ll_spi_tx_u8(reg);
}

void ili9341_write_cmd_param(uint8_t data)
{
	ili9341_ll_dc_high();
	(void)ili9341_ll_spi_tx_u8(data);
}

uint16_t ili9341_read_id4(void)
{
	// Does not work well, need check by Loigc analyzer

	uint8_t dummy, ver, model_h, model_l;

	ili9341_write_cmd(LCD_READ_ID4);
	ili9341_ll_dc_high();
	ili9341_ll_spi_rx_u8(&dummy);
	ili9341_ll_spi_rx_u8(&ver);
	ili9341_ll_spi_rx_u8(&model_h);
	ili9341_ll_spi_rx_u8(&model_l);

	return model_h << 8 | model_l;
}

void ili9341_display_on(void)
{
	ili9341_write_cmd(LCD_DISPLAY_ON);
	ili9341_ll_backlight_on();
}

void ili9341_display_off(void)
{
	ili9341_write_cmd(LCD_DISPLAY_OFF);
	ili9341_ll_backlight_off();
}

void ili9341_reset(void)
{
#if defined(AzureSphere_CA7)
	delay_ms(5);
#elif defined(AzureSphere_CM4)
	vTaskDelay(pdMS_TO_TICKS(5));
#endif

	ili9341_ll_reset_low();

#if defined(AzureSphere_CA7)
	delay_ms(10);
#elif defined(AzureSphere_CM4)
	vTaskDelay(pdMS_TO_TICKS(10));
#endif

	ili9341_ll_reset_high();

#if defined(AzureSphere_CA7)
	delay_ms(120);
#elif defined(AzureSphere_CM4)
	vTaskDelay(pdMS_TO_TICKS(120));
#endif
}

void ili9341_set_window(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1)
{
	ili9341_write_cmd(LCD_COLUMN_ADDR);
	ili9341_write_cmd_param(x0 >> 8);
	ili9341_write_cmd_param(x0);
	ili9341_write_cmd_param(x1 >> 8);
	ili9341_write_cmd_param(x1);

	ili9341_write_cmd(LCD_PAGE_ADDR);
	ili9341_write_cmd_param(y0 >> 8);
	ili9341_write_cmd_param(y0);
	ili9341_write_cmd_param(y1 >> 8);
	ili9341_write_cmd_param(y1);

	ili9341_write_cmd(LCD_GRAM);
}

void ili9341_draw_pixel(uint16_t x, uint16_t y, uint16_t color) 
{
	ili9341_set_window(x, y, x, y);
	ili9341_ll_dc_high();
	ili9341_ll_spi_tx_u16(color);
}

// Graphic part

void ili9341_draw_h_Line(uint16_t x0, uint16_t y0, uint16_t len, uint16_t color) {
	ili9341_fill_rect(x0, y0, len, 1, color);
}

 void ili9341_draw_v_Line(uint16_t x0, uint16_t y0, uint16_t len, uint16_t color) {
	 ili9341_fill_rect(x0, y0, 1, len, color);
}

void ili9341_fill_rect(uint16_t x0, uint16_t y0, uint16_t width, uint16_t height, uint16_t color) {

	uint32_t total_pixel = width * height;
	ili9341_set_window(x0, y0, (x0 + width - 1), (y0 + height - 1));

	ili9341_ll_dc_high();
#if 0
	for (uint32_t i = 0; i < total_pixel; i++) {
		ili9341_ll_spi_tx_u16(color);
	}
#else
	uint32_t i;
	uint16_t local_buf[16] = {0};
	
	for (i = 0; i < 16 ; i++)
		local_buf[i] = color;
	for (i = 0; i < total_pixel; i+=16) {
		ili9341_ll_spi_tx((uint8_t*)local_buf, 32);
	}
#endif
}

void ili9341_clean_screen(uint16_t color)
{
	ili9341_fill_rect(0, 0, ILI9341_LCD_PIXEL_WIDTH, ILI9341_LCD_PIXEL_HEIGHT, color);
}

void LCD_readPixels(uint16_t x0, uint16_t y0, uint16_t *p_color) {

	//uint8_t red, green, blue;

	ili9341_set_window(x0, y0, x0, y0);
	ili9341_write_cmd(LCD_RAMRD);

	// TBC
}

void ili9341_draw_rect(uint16_t x0, uint16_t y0, uint16_t width, uint16_t height, uint16_t color)
{
	ili9341_draw_h_Line(x0, y0, width, color);
	ili9341_draw_h_Line(x0, y0 + height - 1, width, color);
	ili9341_draw_v_Line(x0, y0, height, color);
	ili9341_draw_v_Line(x0 + width - 1, y0, height, color);
}

void ili9341_draw_bitmap(uint16_t x0, uint16_t y0, uint16_t width, uint16_t height, uint8_t *p_bitmap) {

	uint32_t total_bytes = width * height * 2;
	ili9341_set_window(x0, y0, (x0 + width - 1), (y0 + height - 1));

	ili9341_ll_dc_high();
	ili9341_ll_spi_tx(p_bitmap, total_bytes);
}


void ili9341_init(void)
{
	ili9341_ll_init();

	ili9341_reset();

	ili9341_write_cmd(0xCB);
	ili9341_write_cmd_param(0x39);
	ili9341_write_cmd_param(0x2C);
	ili9341_write_cmd_param(0x00);
	ili9341_write_cmd_param(0x34);
	ili9341_write_cmd_param(0x02);

	ili9341_write_cmd(0xCF);
	ili9341_write_cmd_param(0x00);
	ili9341_write_cmd_param(0XC1);
	ili9341_write_cmd_param(0X30);

	ili9341_write_cmd(0xE8);
	ili9341_write_cmd_param(0x85);
	ili9341_write_cmd_param(0x00);
	ili9341_write_cmd_param(0x78);

	ili9341_write_cmd(0xEA);
	ili9341_write_cmd_param(0x00);
	ili9341_write_cmd_param(0x00);

	ili9341_write_cmd(0xED);
	ili9341_write_cmd_param(0x64);
	ili9341_write_cmd_param(0x03);
	ili9341_write_cmd_param(0X12);
	ili9341_write_cmd_param(0X81);

	ili9341_write_cmd(0xF7);
	ili9341_write_cmd_param(0x20);

	ili9341_write_cmd(0xC0);    	//Power control 
	ili9341_write_cmd_param(0x23);   	//VRH[5:0] 

	ili9341_write_cmd(0xC1);    	//Power control 
	ili9341_write_cmd_param(0x10);   	//SAP[2:0];BT[3:0] 

	ili9341_write_cmd(0xC5);    	//VCM control 
	ili9341_write_cmd_param(0x3e);   	//Contrast
	ili9341_write_cmd_param(0x28);

	ili9341_write_cmd(0xC7);
	ili9341_write_cmd_param(0x86);  	    //--

	ili9341_write_cmd(0x36);
#if (ORITENTATION == VERTICAL)
	ili9341_write_cmd_param(0x48);
#elif (ORITENTATION == LANDSCAPE)
	ili9341_write_cmd_param(0x28);
#endif

	ili9341_write_cmd(0x3A);
	ili9341_write_cmd_param(0x55);

	ili9341_write_cmd(0xB1);
	ili9341_write_cmd_param(0x00);
	ili9341_write_cmd_param(0x18);

	ili9341_write_cmd(0xB6);    	// Display Function Control 
	ili9341_write_cmd_param(0x08);
	ili9341_write_cmd_param(0x82);
	ili9341_write_cmd_param(0x27);

	ili9341_write_cmd(0xF2);    	// 3Gamma Function Disable 
	ili9341_write_cmd_param(0x00);

	ili9341_write_cmd(0x26);    	//Gamma curve selected 
	ili9341_write_cmd_param(0x01);

	ili9341_write_cmd(0xE0);    	//Set Gamma 
	ili9341_write_cmd_param(0x0F);
	ili9341_write_cmd_param(0x31);
	ili9341_write_cmd_param(0x2B);
	ili9341_write_cmd_param(0x0C);
	ili9341_write_cmd_param(0x0E);
	ili9341_write_cmd_param(0x08);
	ili9341_write_cmd_param(0x4E);
	ili9341_write_cmd_param(0xF1);
	ili9341_write_cmd_param(0x37);
	ili9341_write_cmd_param(0x07);
	ili9341_write_cmd_param(0x10);
	ili9341_write_cmd_param(0x03);
	ili9341_write_cmd_param(0x0E);
	ili9341_write_cmd_param(0x09);
	ili9341_write_cmd_param(0x00);

	ili9341_write_cmd(0XE1);    	//Set Gamma 
	ili9341_write_cmd_param(0x00);
	ili9341_write_cmd_param(0x0E);
	ili9341_write_cmd_param(0x14);
	ili9341_write_cmd_param(0x03);
	ili9341_write_cmd_param(0x11);
	ili9341_write_cmd_param(0x07);
	ili9341_write_cmd_param(0x31);
	ili9341_write_cmd_param(0xC1);
	ili9341_write_cmd_param(0x48);
	ili9341_write_cmd_param(0x08);
	ili9341_write_cmd_param(0x0F);
	ili9341_write_cmd_param(0x0C);
	ili9341_write_cmd_param(0x31);
	ili9341_write_cmd_param(0x36);
	ili9341_write_cmd_param(0x0F);

	ili9341_write_cmd(0x11);    	//Exit Sleep 

#if defined(AzureSphere_CA7)
	delay_ms(120);
#elif defined(AzureSphere_CM4)
	vTaskDelay(pdMS_TO_TICKS(120));
#endif

	ili9341_clean_screen(0xFFFF);
	ili9341_display_on();
}
