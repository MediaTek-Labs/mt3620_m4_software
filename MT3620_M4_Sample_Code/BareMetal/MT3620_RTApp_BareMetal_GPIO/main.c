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

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>

#include "printf.h"
#include "mt3620.h"
#include "os_hal_uart.h"
#include "os_hal_gpt.h"
#include "os_hal_gpio.h"

/******************************************************************************/
/* Configurations */
/******************************************************************************/
/* UART */
static const uint8_t uart_port_num = OS_HAL_UART_ISU0;

/* GPT */
/* GPT0 for LED blinking */
static const uint8_t gpt_timer_led = OS_HAL_GPT0;
/* GPE3 for Button status scan */
static const uint8_t gpt_timer_button = OS_HAL_GPT3;
/* 500ms, GPT0 clock speed: 1KHz or 32KHz */
static const uint32_t gpt_timer_led_blinking_perios_ms = 500;
/* 100ms, GPT3 clock speed: 1MHz */
static const uint32_t gpt_timer_button_scan_perios_ms = 100000;

/* GPIO */
/* GPIO_8 for LED_Red Control */
static const uint8_t gpio_led_red = OS_HAL_GPIO_8;
/* GPIO_9 for LED_Green Control */
static const uint8_t gpio_led_green = OS_HAL_GPIO_9;
/* GPIO_10 for LED_Blue Control */
static const uint8_t gpio_led_blue = OS_HAL_GPIO_10;
/* GPIO_12 for Button_A Status Sensing */
static const uint8_t gpio_button_a = OS_HAL_GPIO_12;
/* GPIO_13 for Button_B Status Sensing */
static const uint8_t gpio_button_b = OS_HAL_GPIO_13;


/******************************************************************************/
/* Applicaiton Hooks */
/******************************************************************************/
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
static int gpio_output(u8 gpio_no, u8 level)
{
	mtk_os_hal_gpio_set_direction(gpio_no, OS_HAL_GPIO_DIR_OUTPUT);
	mtk_os_hal_gpio_set_output(gpio_no, level);

	return 0;
}

static int gpio_input(u8 gpio_no, os_hal_gpio_data *pvalue)
{
	mtk_os_hal_gpio_set_direction(gpio_no, OS_HAL_GPIO_DIR_INPUT);
	mtk_os_hal_gpio_get_input(gpio_no, pvalue);

	return 0;
}

static void TimerHandlerGpt0(void *cb_data)
{
	static bool ledOn = true;
	uint8_t i;

	/* Toggle LED Status */
	ledOn = !ledOn;
	gpio_output(gpio_led_green, ledOn);

	/* Toggle ISU1~ISU2 GPIO(31~40) */
	/*     ISU1: GPIO31~35 */
	/*     ISU2: GPIO36~40 */
	for (i = OS_HAL_GPIO_31 ; i <= OS_HAL_GPIO_40 ; i++)
		gpio_output(i, ledOn);
}

static void TimerHandlerGpt3(void *cb_data)
{
	os_hal_gpio_data button_status;

	/* Set LED_Blue according to Button_A */
	gpio_input(gpio_button_a, &button_status);
	if (button_status)
		gpio_output(gpio_led_blue, OS_HAL_GPIO_DATA_HIGH);
	else
		gpio_output(gpio_led_blue, OS_HAL_GPIO_DATA_LOW);

	/* Set LED_Red according to Button_B */
	gpio_input(gpio_button_b, &button_status);
	if (button_status)
		gpio_output(gpio_led_red, OS_HAL_GPIO_DATA_HIGH);
	else
		gpio_output(gpio_led_red, OS_HAL_GPIO_DATA_LOW);

	/* Restart GPT3 since it's one-shot mode. */
	mtk_os_hal_gpt_restart(gpt_timer_button);
}

_Noreturn void RTCoreMain(void)
{
	struct os_gpt_int gpt0_int;
	struct os_gpt_int gpt3_int;

	/* Init Vector Table */
	NVIC_SetupVectorTable();

	/* Init UART */
	mtk_os_hal_uart_ctlr_init(uart_port_num);
	printf("\nUART Inited (port_num=%d)\n", uart_port_num);

	/* Init GPT */
	mtk_os_hal_gpt_init();

	/* Init GPT0 for LED blinking, repeat mode. */
	gpt0_int.gpt_cb_hdl = TimerHandlerGpt0;
	gpt0_int.gpt_cb_data = NULL;
	mtk_os_hal_gpt_config(gpt_timer_led, false, &gpt0_int);
	mtk_os_hal_gpt_reset_timer(gpt_timer_led,
					gpt_timer_led_blinking_perios_ms, true);
	mtk_os_hal_gpt_start(gpt_timer_led);

	/* Init GPT3 for button status monitor, one-shot mode. */
	gpt3_int.gpt_cb_hdl = TimerHandlerGpt3;
	gpt3_int.gpt_cb_data = NULL;
	mtk_os_hal_gpt_config(gpt_timer_button, false, &gpt3_int);
	mtk_os_hal_gpt_reset_timer(gpt_timer_button,
					gpt_timer_button_scan_perios_ms, false);
	mtk_os_hal_gpt_start(gpt_timer_button);

	for (;;)
		__asm__("wfi");
}

