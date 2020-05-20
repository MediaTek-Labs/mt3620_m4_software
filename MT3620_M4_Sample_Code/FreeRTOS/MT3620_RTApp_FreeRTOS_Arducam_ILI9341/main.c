/*
 * (C) 2005-2020 MediaTek Inc. All rights reserved.
 *
 * Copyright Statement:
 *
 * This MT3620 driver software/firmware and related documentation
 * ("MediaTek Software") are protected under relevant copyright laws.
 * The information contained herein is confidential and proprietary to
 * MediaTek Inc. ("MediaTek"). You may only use, reproduce, modify, or
 * distribute (as applicable) MediaTek Software if you have agreed to and been
 * bound by this Statement and the applicable license agreement with MediaTek
 * ("License Agreement") and been granted explicit permission to do so within
 * the License Agreement ("Permitted User"). If you are not a Permitted User,
 * please cease any access or use of MediaTek Software immediately.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT MEDIATEK SOFTWARE RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE
 * PROVIDED TO RECEIVER ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS
 * ANY AND ALL WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH MEDIATEK SOFTWARE, AND RECEIVER AGREES TO
 * LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO MEDIATEK SOFTWARE RELEASED
 * HEREUNDER WILL BE ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY
 * RECEIVER TO MEDIATEK DURING THE PRECEDING TWELVE (12) MONTHS FOR SUCH
 * MEDIATEK SOFTWARE AT ISSUE.
 */

#include "FreeRTOS.h"
#include "task.h"
#include "printf.h"
#include "mt3620.h"

#include "os_hal_gpio.h"
#include "os_hal_uart.h"
#include "os_hal_i2c.h"
#include "os_hal_spim.h"

#include "arducam_driver.h"
#include "ili9341.h"
#include "text.h"

/******************************************************************************/
/* Configurations */
/******************************************************************************/
/* For Debug Log */
static const UART_PORT uart_port_num = OS_HAL_UART_PORT0;
static const uint32_t uart_baudrate = 115200;

/* For Arducam SPI */
static spim_num spi_master_port_num_arducam = OS_HAL_SPIM_ISU1;
static uint32_t spi_master_speed_khz_arducam = 8000;	/* 8MHz */

/* For Arducam I2c */
static const i2c_num i2c_master_port_num_arducam = OS_HAL_I2C_ISU0;
static const enum i2c_speed_kHz i2c_master_speed_arducam = I2C_SCL_1000kHz;
static const uint8_t i2c_arducam_write_addr = 0x60>>1;

/* For ILI9341 SPI */
spim_num spi_master_port_num_ili9341 = OS_HAL_SPIM_ISU3;
uint32_t spi_master_speed_khz_ili9341 = 40000;	/* 40MHz */

/* For ILI9341 RST/DC/BL */
const os_hal_gpio_pin gpio_ili9341_rst = OS_HAL_GPIO_0;
const os_hal_gpio_pin gpio_ili9341_dc = OS_HAL_GPIO_1;
const os_hal_gpio_pin gpio_ili9341_bl = OS_HAL_GPIO_2;

/* For on-board buttons */
static const os_hal_gpio_pin gpio_button_a = OS_HAL_GPIO_12;
static const os_hal_gpio_pin gpio_button_b = OS_HAL_GPIO_13;

/* APP Stack Size */
#define APP_STACK_SIZE_BYTES (1024 / 4)

/* SPI variables */
#define SPI_MAX_BUF_SIZE 32
static struct mtk_spi_config spi_default_config_arducam = {
	.cpol = SPI_CPOL_0,
	.cpha = SPI_CPHA_0,
	.rx_mlsb = SPI_MSB,
	.tx_mlsb = SPI_MSB,
	.slave_sel = SPI_SELECT_DEVICE_0,
};
struct mtk_spi_config spi_default_config_ili9341 = {
	.cpol = SPI_CPOL_0,
	.cpha = SPI_CPHA_0,
	.rx_mlsb = SPI_MSB,
	.tx_mlsb = SPI_MSB,
	.slave_sel = SPI_SELECT_DEVICE_0,
};

/* Commands */
static uint8_t cmd_arducam_capture_single;
static uint8_t cmd_arducam_capture_continuous;

/******************************************************************************/
/* Applicaiton Hooks */
/******************************************************************************/
/* Hook for "stack over flow". */
void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName)
{
	printf("%s: %s\n", __func__, pcTaskName);
}

/* Hook for "memory allocation failed". */
void vApplicationMallocFailedHook(void)
{
	printf("%s\n", __func__);
}

/* Hook for "printf". */
void _putchar(char character)
{
	mtk_os_hal_uart_put_char(uart_port_num, character);
	if (character == '\n')
		mtk_os_hal_uart_put_char(uart_port_num, '\r');
}

/******************************************************************************/
/* Functions */
/******************************************************************************/
void arducam_ili9341_task(void *pParameters)
{
	vTaskDelay(pdMS_TO_TICKS(1000));

	printf("Arducam_ILI9341 Task Started.\n");

	/* Init Arducam */
	arducam_init(spi_master_port_num_arducam, spi_master_speed_khz_arducam, SPI_MAX_BUF_SIZE,
			&spi_default_config_arducam, i2c_master_port_num_arducam, i2c_master_speed_arducam,
			i2c_arducam_write_addr);

	/* Init ILI9341 */
	ili9341_init();

	/* draw text and a rect of display area */
	ili9341_clean_screen(BLACK);
	lcd_set_text_color(YELLOW);
	lcd_set_text_bg_color(BLACK);
	lcd_set_text_size(2);
	lcd_set_text_cursor(5, 10);
	lcd_display_string("Button A: Single Capture");
	lcd_set_text_cursor(5, 50);
	lcd_display_string("Button B: Repeat Capture");

	/* Init command status */
	cmd_arducam_capture_single = 0;
	cmd_arducam_capture_continuous = 0;

	while (1) {
		vTaskDelay(pdMS_TO_TICKS(10));

		/* Handle cmd: Single Capture */
		if (cmd_arducam_capture_single) {
			cmd_arducam_capture_single = 0;
			/* Read captured data from Arducam and write to ILI9341 */
			/* Captured data: BMP(320x240) or JPEG (160x120) */
			arducam_capture();
		}

		/* Handle cmd: Continuous Capture */
		if (cmd_arducam_capture_continuous)
			/* Read captured data from Arducam and write to ILI9341 */
			/* Captured data: BMP(320x240) or JPEG (160x120) */
			arducam_capture();
	}
}

static void button_task(void *pParameters)
{
	static os_hal_gpio_data last_button_a_status = OS_HAL_GPIO_DATA_HIGH;
	static os_hal_gpio_data last_button_b_status = OS_HAL_GPIO_DATA_HIGH;
	os_hal_gpio_data button_a_status, button_b_status;

	printf("Button Task Started (Monitoring on-board button status).\n");
	while (1) {
		vTaskDelay(pdMS_TO_TICKS(10));

		/* Get button status from GPIO */
		mtk_os_hal_gpio_get_input(gpio_button_a, &button_a_status);
		mtk_os_hal_gpio_get_input(gpio_button_b, &button_b_status);

		/* Update button status */
		if (button_a_status == OS_HAL_GPIO_DATA_HIGH &&
			last_button_a_status == OS_HAL_GPIO_DATA_LOW) {
			printf("Button A Click\n");
			cmd_arducam_capture_single = 1;
			cmd_arducam_capture_continuous = 0;
		}
		if (button_b_status == OS_HAL_GPIO_DATA_HIGH &&
			last_button_b_status == OS_HAL_GPIO_DATA_LOW) {
			printf("Button B Click\n");
			cmd_arducam_capture_single = 0;
			cmd_arducam_capture_continuous = 1;
		}
		last_button_a_status = button_a_status;
		last_button_b_status = button_b_status;
	}
}

_Noreturn void RTCoreMain(void)
{
	/* Setup Vector Table */
	NVIC_SetupVectorTable();

	/* Init Debug UART port */
	mtk_os_hal_uart_ctlr_init(uart_port_num);
	mtk_os_hal_uart_set_baudrate(uart_port_num, uart_baudrate);

	/* Init GPIO */
	mtk_os_hal_gpio_request(gpio_button_a);
	mtk_os_hal_gpio_set_direction(gpio_button_a, OS_HAL_GPIO_DIR_INPUT);
	mtk_os_hal_gpio_request(gpio_button_b);
	mtk_os_hal_gpio_set_direction(gpio_button_b, OS_HAL_GPIO_DIR_INPUT);

	printf("\nFreeRTOS Arducam Demo\n");

	/* Create Arducam_ILI9341 Task */
	xTaskCreate(arducam_ili9341_task, "Arducam_ILI9341 Task", APP_STACK_SIZE_BYTES, NULL, 5, NULL);

	/* Create Button Task, check On-board button status */
	xTaskCreate(button_task, "Button Task", APP_STACK_SIZE_BYTES, NULL, 4, NULL);

	vTaskStartScheduler();
	for (;;)
		__asm__("wfi");
}

