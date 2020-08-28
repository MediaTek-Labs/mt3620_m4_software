/*
 * (C) 2005-2019 MediaTek Inc. All rights reserved.
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

#include "FreeRTOS.h"
#include "task.h"
#include "printf.h"
#include "mt3620.h"

#include "os_hal_gpio.h"
#include "os_hal_uart.h"

#ifdef __cplusplus
extern "C" {
#endif
void vApplicationStackOverflowHook(TaskHandle_t xTask, char* pcTaskName);
void vApplicationMallocFailedHook(void);
_Noreturn void RTCoreMain(void);
typedef void (*InitFunc)(void);
extern InitFunc __init_array_start;
extern InitFunc __init_array_end;
#ifdef __cplusplus
}
#endif

/******************************************************************************/
/* Configurations */
/******************************************************************************/
static const UART_PORT uart_port_num = OS_HAL_UART_ISU0;
static const os_hal_gpio_pin gpio_led_green = OS_HAL_GPIO_9;

#define APP_STACK_SIZE_BYTES		(1024 / 4)
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
/* Class */
/******************************************************************************/
class DigitalOut
{
private:
	os_hal_gpio_pin _Pin;

public:
	DigitalOut(os_hal_gpio_pin pin);
	void Write(int value);
};

DigitalOut::DigitalOut(os_hal_gpio_pin pin)
{
	_Pin = pin;
	mtk_os_hal_gpio_set_direction(_Pin, OS_HAL_GPIO_DIR_OUTPUT);
}

void DigitalOut::Write(int value)
{
	mtk_os_hal_gpio_set_output(_Pin, value ? OS_HAL_GPIO_DATA_HIGH : OS_HAL_GPIO_DATA_LOW);
}

/******************************************************************************/
/* Functions */
/******************************************************************************/
static void blink_task(void* pParameters)
{
	DigitalOut led(gpio_led_green);

	printf("Blink Task Started\n");

	while (1)
	{
		led.Write(0);
		vTaskDelay(pdMS_TO_TICKS(100));
		led.Write(1);
		vTaskDelay(pdMS_TO_TICKS(100));
	}
}

_Noreturn void CcMain(void)
{
	/* Init UART */
	mtk_os_hal_uart_ctlr_init(uart_port_num);

	printf("\nFreeRTOS C++ Blink Demo\n");

	/* Create Blink Task */
	xTaskCreate(blink_task, "Blink Task", APP_STACK_SIZE_BYTES, NULL, 4, NULL);

	vTaskStartScheduler();
	for (;;)
		__asm__("wfi");
}

_Noreturn void RTCoreMain(void)
{
	// Setup vector table
	NVIC_SetupVectorTable();

	// Call global constructors
	for (InitFunc* func = &__init_array_start; func < &__init_array_end; ++func) (*func)();

	CcMain();
}
