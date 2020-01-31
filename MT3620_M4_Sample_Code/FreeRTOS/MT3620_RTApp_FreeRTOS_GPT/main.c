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

#include "os_hal_uart.h"
#include "os_hal_gpt.h"

/******************************************************************************/
/* Configurations */
/******************************************************************************/
static const uint8_t uart_port_num = OS_HAL_UART_ISU0;
static const uint8_t gpt_timer_0_id = OS_HAL_GPT0;		// OS_HAL_GPT0 clock speed: 1KHz or 32KHz
static const uint32_t gpt_timer_0_interval = 1000;		// 1000ms
static const uint8_t gpt_timer_3_id = OS_HAL_GPT3;		// OS_HAL_GPT3 clock speed: 1MHz
static const uint32_t gpt_timer_3_interval = 5000000;	// 5000ms

#define APP_STACK_SIZE_BYTES (1024 / 4)

/******************************************************************************/
/* Applicaiton Hooks */
/******************************************************************************/
// Hook for "stack over flow".
void vApplicationStackOverflowHook(TaskHandle_t xTask, char* pcTaskName)
{
	printf("%s: %s\n", __func__, pcTaskName);
}

// Hook for "memory allocation failed".
void vApplicationMallocFailedHook(void)
{
	printf("%s\n", __func__);
}

// Hook for "printf".
void _putchar(char character)
{
	mtk_os_hal_uart_put_char(uart_port_num, character);
	if (character == '\n')
		mtk_os_hal_uart_put_char(uart_port_num, '\r');
}

/******************************************************************************/
/* Functions */
/******************************************************************************/
static void TimerHandlerGpt0(void* cb_data)
{
	extern volatile uint32_t sys_tick_in_ms;
	printf("%s (SysTick=%ld)\n", __func__, sys_tick_in_ms);
}

static void TimerHandlerGpt3(void* cb_data)
{
	extern volatile uint32_t sys_tick_in_ms;
	printf("%s (SysTick=%ld)\n", __func__, sys_tick_in_ms);
	mtk_os_hal_gpt_restart(gpt_timer_3_id);
}

static void gpt_task(void* pParameters)
{
	struct os_gpt_int gpt0_int;
	struct os_gpt_int gpt1_int;

	gpt0_int.gpt_cb_hdl = TimerHandlerGpt0;
	gpt0_int.gpt_cb_data = NULL;
	gpt1_int.gpt_cb_hdl = TimerHandlerGpt3;
	gpt1_int.gpt_cb_data = NULL;

	// Start GPT0
	printf("    Set GPT0 AutoRepeat = true, Timeout = %dms\n", (int)gpt_timer_0_interval);
	mtk_os_hal_gpt_config(gpt_timer_0_id, false, &gpt0_int);
	mtk_os_hal_gpt_reset_timer(gpt_timer_0_id, gpt_timer_0_interval, true);
	mtk_os_hal_gpt_start(gpt_timer_0_id);

	// Start GPT3
	printf("    Set GPT3 AutoRepeat = false, Timeout = %dms\n", (int)(gpt_timer_3_interval/1000));
	mtk_os_hal_gpt_config(gpt_timer_3_id, false, &gpt1_int);
	mtk_os_hal_gpt_reset_timer(gpt_timer_3_id, gpt_timer_3_interval, false);
	mtk_os_hal_gpt_start(gpt_timer_3_id);

	// Note, only GPT0 and GPT1 supports repeat mode.
	// For more information, please refer to https://support.mediatek.com/AzureSphere/mt3620/M4_API_Reference_Manual/group___g_p_t.html

	while(1) {
		vTaskDelay(pdMS_TO_TICKS(10));
	}
}

_Noreturn void RTCoreMain(void)
{
	// Setup Vector Table
	NVIC_SetupVectorTable();

	// Init UART
	mtk_os_hal_uart_ctlr_init(uart_port_num);
	printf("\nFreeRTOS GPT demo\n");

	// Init GPT
	mtk_os_hal_gpt_init();

	// Create GPT Task
	xTaskCreate(gpt_task, "GPT Task", APP_STACK_SIZE_BYTES, NULL, 4, NULL);

	vTaskStartScheduler();
	for (;;) {
		__asm__("wfi");
	}
}

