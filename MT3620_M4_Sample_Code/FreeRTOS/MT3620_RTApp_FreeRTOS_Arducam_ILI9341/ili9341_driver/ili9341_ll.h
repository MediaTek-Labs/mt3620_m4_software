#ifndef __ILI9341_LL_H
#define __ILI9341_LL_H

#include <stdint.h>

int ili9341_ll_init(void);

void ili9341_ll_reset_low(void);
void ili9341_ll_reset_high(void);

void ili9341_ll_dc_low(void);
void ili9341_ll_dc_high(void);

void ili9341_ll_backlight_off(void);
void ili9341_ll_backlight_on(void);

int ili9341_ll_spi_tx_u8(uint8_t data);
int ili9341_ll_spi_rx_u8(uint8_t* data);
int ili9341_ll_spi_tx_u16(uint16_t data);
int ili9341_ll_spi_tx(uint8_t* p_data, uint32_t len);
#endif /* __ILI9341_LL_H */
