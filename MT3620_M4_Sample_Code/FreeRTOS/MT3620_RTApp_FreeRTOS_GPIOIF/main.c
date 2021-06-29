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
#include "os_hal_gpioif.h"

/******************************************************************************/
/* Configurations */
/******************************************************************************/
static const UART_PORT uart_port_num = OS_HAL_UART_ISU0;

/* This sample code demostrates the usage of GPIOIF (external interrupt HW counter).
 *    GPIO_12 (BTN_A on the AVNET/Seeed board) is used as GPIOIF GPIO_0.
 *    GPIO_13 (BTN_B on the AVNET/Seeed board) is used to GPIOIF GPIO_1.
 *
 * GPIOIF / GPIO Mapping
 *    GPIO_GROUP_0/GPIO_0: GPIO_0
 *    GPIO_GROUP_0/GPIO_1: GPIO_1
 *    GPIO_GROUP_0/GPIO_2: GPIO_2
 *    GPIO_GROUP_1/GPIO_0: GPIO_4
 *    GPIO_GROUP_1/GPIO_1: GPIO_5
 *    GPIO_GROUP_1/GPIO_2: GPIO_6
 *    GPIO_GROUP_2/GPIO_0: GPIO_8
 *    GPIO_GROUP_2/GPIO_1: GPIO_9
 *    GPIO_GROUP_2/GPIO_2: GPIO_10
 *    GPIO_GROUP_3/GPIO_0: GPIO_12
 *    GPIO_GROUP_3/GPIO_1: GPIO_13
 *    GPIO_GROUP_3/GPIO_2: GPIO_14
 *    GPIO_GROUP_4/GPIO_0: GPIO_16
 *    GPIO_GROUP_4/GPIO_1: GPIO_17
 *    GPIO_GROUP_4/GPIO_2: GPIO_18
 *    GPIO_GROUP_5/GPIO_0: GPIO_20
 *    GPIO_GROUP_5/GPIO_1: GPIO_21
 *    GPIO_GROUP_5/GPIO_2: GPIO_22
 *
 * GPIOIF Counter Modes:
 *    Up/Down Mode
 *        GPIO_0(Input): Count Up
 *        GPIO_1(Input): Count Down
 *        GPIO_2(Input): Reset
 *    Direction Mode
 *        GPIO_0(Input): Count Pulse
 *        GPIO_1(Input): Direction Indicator
 *        GPIO_2(Input): Reset
 *    Quadrature Mode
 *        Count Up: GPIO_0 Pulse before GPIO_1
 *        Count Down: GPIO_0 Pulse after GPIO_1
 *        GPIO_2(Input): Reset
*/

#define GPIOIF_MODE 0			/* 0:Up/Down Mode, 1:Direction Mode, 2:Quadrature Mode */
#define GPIOIF_GROUP GPIOIF_GROUP_3
#define GPIOIF_GPIO_0 0			/* GPIO_GROUP_3/GPIO_0 = GPIO_12 = BTN_A */
#define GPIOIF_GPIO_1 1			/* GPIO_GROUP_3/GPIO_1 = GPIO_13 = BTN_B */
#define GPIOIF_ENABLE 1
#define GPIOIF_DISABLE 0
#define GPIOIF_INTR_CLEAR 1
#define GPIOIF_COUNTER_HIGH_LIMIT 110	/* GPIOIF Counter High Limit */
#define GPIOIF_COUNTER_LOW_LIMIT 90	/* GPIOIF Counter Low Limit */
#define GPIOIF_COUNTER_RESET_VALUE 100	/* GPIOIF Counter Default Val */
#define GPIOIF_CONTROL_SETTING	3	/* 1: Count on Rising Edge, 2: Count on Falling Edge, 0/3: Count on Both */
#define GPIOIF_DEGLITCH_MIN_P 128	/* 0~16383 */
#define GPIOIF_DEGLITCH_INIT_V 0	/* 0 or 1 */
#define GPIOIF_RESET_BY_GPIO_2	0	/* 0: Disable GPIO_2 Reset, 1:Enable GPIO_2 Reset */

#define APP_STACK_SIZE_BYTES (1024 / 4)

static volatile u8 gpioif_callback_asserted;
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

/******************************************************************************/
/* Functions */
/******************************************************************************/
int _gpioif_callback(void *user_data)
{
	int gpioif_status = 0;
	u32 pvalue;

	if (gpioif_callback_asserted == 0) {
		printf("    GPIOIF Callback Triggered, ");

		/* Read Event Count */
		gpioif_status = mtk_os_hal_gpioif_read_gpio_event_count(GPIOIF_GROUP, &pvalue);
		if (gpioif_status)
			printf("mtk_os_hal_gpioif_read_gpio_event_count failed (%d)\n", gpioif_status);
		else
			printf("Event Count=%d", pvalue);

		printf(", Reset GPIOIF\n\n");

		/* GPIOIF global reset */
		gpioif_status = mtk_os_hal_gpioif_global_cr_reset(GPIOIF_GROUP);
		if (gpioif_status) {
			printf("mtk_os_hal_gpioif_ctlr_init failed (%d)\n", gpioif_status);
			return -1;
		}

		gpioif_callback_asserted = 1;
	}

	return 0;
}

void _gpioif_configure(void)
{
	int gpioif_status = 0;

	printf("Config/Init GPIOIF\n");

	/* GPIOIF global reset */
	gpioif_status = mtk_os_hal_gpioif_global_cr_reset(GPIOIF_GROUP);
	if (gpioif_status) {
		printf("mtk_os_hal_gpioif_ctlr_init failed (%d)\n", gpioif_status);
		goto exit_failed;
	}

	/* Init GPIOIF */
	gpioif_status = mtk_os_hal_gpioif_ctlr_init(GPIOIF_GROUP);
	if (gpioif_status) {
		printf("mtk_os_hal_gpioif_ctlr_init failed (%d)\n", gpioif_status);
		goto exit_failed;
	}

	/* Register Callback */
	gpioif_status = mtk_os_hal_gpioif_int_callback_register(GPIOIF_GROUP, _gpioif_callback, NULL);
	if (gpioif_status) {
		printf("mtk_os_hal_gpioif_int_callback_register failed (%d)\n", gpioif_status);
		goto exit_failed;
	}

	/* GPIOIF HW Reset */
	gpioif_status = mtk_os_hal_gpioif_hardware_reset(GPIOIF_GROUP, MTK_OS_GPIOIF_EVENT_CAP_COUNTER_MODE,
								GPIOIF_RESET_BY_GPIO_2);
	if (gpioif_status) {
		printf("mtk_os_hal_gpioif_hardware_reset failed (%d)\n", gpioif_status);
		goto exit_failed;
	}

	/* DeGlitch */
	gpioif_status = mtk_os_hal_gpioif_de_glitch(GPIOIF_GROUP, GPIOIF_GPIO_0, GPIOIF_ENABLE, GPIOIF_DEGLITCH_MIN_P,
							GPIOIF_DEGLITCH_INIT_V);
	if (gpioif_status) {
		printf("mtk_os_hal_gpioif_de_glitch failed (%d)\n", gpioif_status);
		goto exit_failed;
	}
	gpioif_status = mtk_os_hal_gpioif_de_glitch(GPIOIF_GROUP, GPIOIF_GPIO_1, GPIOIF_ENABLE, GPIOIF_DEGLITCH_MIN_P,
							GPIOIF_DEGLITCH_INIT_V);
	if (gpioif_status) {
		printf("mtk_os_hal_gpioif_de_glitch failed (%d)\n", gpioif_status);
		goto exit_failed;
	}

	/* Set Limit Comparator */
	gpioif_status = mtk_os_hal_gpioif_limit_comparator(GPIOIF_GROUP, GPIOIF_NOT_SA_LIMIT_V,
								GPIOIF_INTERRUPT_BOTH_V);
	if (gpioif_status) {
		printf("mtk_os_hal_gpioif_limit_comparator failed (%d)\n", gpioif_status);
		goto exit_failed;
	}

	/* Set GPIOIF Mode */
	if (GPIOIF_MODE == 0) {
		/* Set Up Down Mode - GPIO_0:Count Up, GPIO_1:Count Down*/
		gpioif_status = mtk_os_hal_gpioif_set_updown_mode(GPIOIF_GROUP, GPIOIF_CONTROL_SETTING,
								GPIOIF_COUNTER_LOW_LIMIT, GPIOIF_COUNTER_HIGH_LIMIT,
								GPIOIF_COUNTER_RESET_VALUE, GPIOIF_CLOCK_200MHZ);
	} else if (GPIOIF_MODE == 1) {
		/* Set Direction Mode - GPIO_0:Count Pulse, GPIO_1: Direction Indicator*/
		gpioif_status = mtk_os_hal_gpioif_set_direction_mode(GPIOIF_GROUP, GPIOIF_CONTROL_SETTING,
								GPIOIF_COUNTER_LOW_LIMIT, GPIOIF_COUNTER_HIGH_LIMIT,
								GPIOIF_COUNTER_RESET_VALUE, GPIOIF_CLOCK_200MHZ);
	} else if (GPIOIF_MODE == 2) {
		/* Set Quadrature Mode - Count Up: GPIO_0 pulse before GPIO_1, Count Down: GPIO_0 pulse after GPIO_1*/
		gpioif_status = mtk_os_hal_gpioif_set_quadrature_mode(GPIOIF_GROUP, GPIOIF_CONTROL_SETTING,
								GPIOIF_COUNTER_LOW_LIMIT, GPIOIF_COUNTER_HIGH_LIMIT,
								GPIOIF_COUNTER_RESET_VALUE, GPIOIF_CLOCK_200MHZ);
	} else {
		printf("GPIOIF Mode not supported! (Mode value should be 0~3)\n");
		goto exit_failed;
	}
	if (gpioif_status) {
		printf("mtk_os_hal_gpioif_set_mode failed (%d)\n", gpioif_status);
		goto exit_failed;
	}

	/* Enable Interrupt */
	gpioif_status = mtk_os_hal_gpioif_interrupt_control(GPIOIF_GROUP, GPIOIF_ENABLE, GPIOIF_INTR_CLEAR,
								IRQ_GPIOIF_ALL_ENABLE);
	if (gpioif_status) {
		printf("mtk_os_hal_gpioif_interrupt_control failed (%d)\n", gpioif_status);
		goto exit_failed;
	}

	return;

exit_failed:
	printf("gpioif test fail!\n");
}

void gpioif_task(void *pParameters)
{
	int gpioif_status = 0;
	u32 pvalue;

	/* Configure GPIOIF */
	_gpioif_configure();

	while (1) {
		/* Delay for 1000ms */
		vTaskDelay(pdMS_TO_TICKS(1000));

		/* Reconfigure GPIOIF if callback asserted*/
		if (gpioif_callback_asserted == 1) {
			_gpioif_configure();
			gpioif_callback_asserted = 0;
		}

		/* Read Event Count */
		gpioif_status = mtk_os_hal_gpioif_read_gpio_event_count(GPIOIF_GROUP, &pvalue);
		if (!gpioif_status) {
			printf("    Read Event Count: %d\n", pvalue);
		} else {
			printf("mtk_os_hal_gpioif_read_gpio_event_count failed (%d)\n", gpioif_status);
			return;
		}
	}
}


_Noreturn void RTCoreMain(void)
{
	/* Setup Vector Table */
	NVIC_SetupVectorTable();

	/* Init UART */
	mtk_os_hal_uart_ctlr_init(uart_port_num);

	printf("\n\n\nFreeRTOS GPIOIF Demo\n\n");

	/* Create GPIOIF Task */
	xTaskCreate(gpioif_task, "GPIOIF Task", APP_STACK_SIZE_BYTES, NULL, 4, NULL);

	vTaskStartScheduler();
	for (;;)
		__asm__("wfi");
}

