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

#include "os_hal_uart.h"
#include "os_hal_gpio.h"
#include "os_hal_eint.h"

/****************************************************************************/
/* Configurations */
/****************************************************************************/
static const uint8_t uart_port_num = OS_HAL_UART_ISU0;

#define GPIO_OUTPUT_LED_RED		OS_HAL_GPIO_8
#define GPIO_OUTPUT_LED_GREEN	OS_HAL_GPIO_9
#define GPIO_INTERRUPT_BTN_A	OS_HAL_GPIO_12
#define GPIO_INTERRUPT_BTN_B	OS_HAL_GPIO_13

#define APP_STACK_SIZE_BYTES (1024 / 4)

/****************************************************************************/
/* Applicaiton Hooks */
/****************************************************************************/
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

/****************************************************************************/
/* Functions */
/****************************************************************************/
void eint_callback_btn_a(void)
{
	static uint32_t cbk_counter_a;

	if (cbk_counter_a % 2)
		mtk_os_hal_gpio_set_output(GPIO_OUTPUT_LED_RED, OS_HAL_GPIO_DATA_HIGH);
	else
		mtk_os_hal_gpio_set_output(GPIO_OUTPUT_LED_RED, OS_HAL_GPIO_DATA_LOW);

	printf("    Botton_A Callback (%ld)\n", cbk_counter_a);
	cbk_counter_a++;
}

void eint_callback_btn_b(void)
{
	static uint32_t cbk_counter_b;

	if (cbk_counter_b % 2)
		mtk_os_hal_gpio_set_output(GPIO_OUTPUT_LED_GREEN, OS_HAL_GPIO_DATA_HIGH);
	else
		mtk_os_hal_gpio_set_output(GPIO_OUTPUT_LED_GREEN, OS_HAL_GPIO_DATA_LOW);

	printf("    Botton_B Callback (%ld)\n", cbk_counter_b);
	cbk_counter_b++;
}

void EINTTask(void *pParameters)
{
	printf("Please press Button_A or Button_B for EINT demostration.\n");

	/* Configure the LED */
	mtk_os_hal_gpio_set_direction(GPIO_OUTPUT_LED_RED, OS_HAL_GPIO_DIR_OUTPUT);
	mtk_os_hal_gpio_set_output(GPIO_OUTPUT_LED_RED, OS_HAL_GPIO_DATA_HIGH);
	mtk_os_hal_gpio_set_direction(GPIO_OUTPUT_LED_GREEN, OS_HAL_GPIO_DIR_OUTPUT);
	mtk_os_hal_gpio_set_output(GPIO_OUTPUT_LED_GREEN, OS_HAL_GPIO_DATA_HIGH);

	/* Register EINT and set/enable debounce for BTN_A. */
	mtk_os_hal_eint_register(GPIO_INTERRUPT_BTN_A, HAL_EINT_EDGE_FALLING_AND_RISING, eint_callback_btn_a);
	mtk_os_hal_eint_set_debounce(GPIO_INTERRUPT_BTN_A, OS_HAL_EINT_DB_TIME_1);

	/* Register EINT and set/enable debounce for BTN_B. */
	mtk_os_hal_eint_register(GPIO_INTERRUPT_BTN_B, HAL_EINT_EDGE_FALLING, eint_callback_btn_b);
	mtk_os_hal_eint_set_debounce(GPIO_INTERRUPT_BTN_B, OS_HAL_EINT_DB_TIME_1);

	while (1) {
		/* Do nothing here, the external interrupt is handled in the callback functions. */
		vTaskDelay(pdMS_TO_TICKS(1));
	}

	mtk_os_hal_eint_unregister(GPIO_INTERRUPT_BTN_A);
	mtk_os_hal_eint_unregister(GPIO_INTERRUPT_BTN_B);
}

_Noreturn void RTCoreMain(void)
{
	/* Setup Vector Table */
	NVIC_SetupVectorTable();

	/* Init UART */
	mtk_os_hal_uart_ctlr_init(uart_port_num);
	printf("\nFreeRTOS EINT Demo\n");

	/* Create EINT Task */
	xTaskCreate(EINTTask, "EINT Task", APP_STACK_SIZE_BYTES, NULL, 4, NULL);

	vTaskStartScheduler();
	for (;;)
		__asm__("wfi");
}

