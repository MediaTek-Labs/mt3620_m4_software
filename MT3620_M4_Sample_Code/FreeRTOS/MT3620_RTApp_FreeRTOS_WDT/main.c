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
#include "os_hal_wdt.h"

/******************************************************************************/
/* Configurations */
/******************************************************************************/
static const uint8_t uart_port_num = OS_HAL_UART_ISU0;

#define WDT_TIMEOUT_SEC 10	/* Watchdog timeout value, 10 seconds */
#define WDT_TEST_CASE_1_COUNTER 10

#define APP_STACK_SIZE_BYTES (1024 / 4)

#define COLOR_YELLOW        "\033[1;33m"
#define COLOR_NONE          "\033[m"

/* Global variables with non-zero init value will be stored in .data section. */
/* .data section will NOT be reinitialized after watchdog reset. */
int g_variable_with_non_zero_init_value = 100;

/* Global variables without non-zero init value will stored in .bss section. */
/* .bss section will be reinitialized(set to zero) after watchdog reset. */
int g_variable_without_non_zero_init_value;

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
static void wdt_irq_handle(void *unused)
{
	printf("\nWatchdog Timeout !!!!!\n");
	printf("Trigger WDT HW Reset\n");
	mtk_os_hal_wdt_hw_reset();
}

void wdt_task(void *pParameters)
{
	uint8_t test_case_1_counter = WDT_TEST_CASE_1_COUNTER;
	enum os_wdt_rst_sta rst_sta;
	struct os_wdt_int wdt_int = {
		.wdt_cb_hdl = wdt_irq_handle,
		.wdt_cb_data = NULL,
	};

	/* WDT should be be inited before reset status can be obtained. */
	mtk_os_hal_wdt_init();
	rst_sta = mtk_os_hal_wdt_get_reset_status();
	printf("Boot reset status = %d("COLOR_YELLOW"%s"COLOR_NONE")\n", rst_sta,
					rst_sta == OS_WDT_NONE_RST ? "NONE" :
					rst_sta == OS_WDT_SW_RST ? "SW_RST" :
					rst_sta == OS_WDT_HW_RST ? "HW_RST" :
					"Unknown!");
	if ((rst_sta != OS_WDT_NONE_RST) && (rst_sta != OS_WDT_SW_RST) &&
		(rst_sta != OS_WDT_HW_RST)) {
		printf("Unknown reset status(%d)!, WDT test stopped\n",
			rst_sta);
		return;
	}

	printf("g_variable_with_non_zero_init_value++ : "COLOR_YELLOW"%d"COLOR_NONE"\n",
		++g_variable_with_non_zero_init_value);
	printf("g_variable_without_non_zero_init_value++ : "COLOR_YELLOW"%d"COLOR_NONE"\n",
		++g_variable_without_non_zero_init_value);

	mtk_os_hal_wdt_set_timeout(WDT_TIMEOUT_SEC);
	mtk_os_hal_wdt_register_irq(&wdt_int);
	mtk_os_hal_wdt_config(OS_WDT_TRIGGER_IRQ);
	mtk_os_hal_wdt_enable();
	printf("Watchdog timer is inited as %d seconds\n", WDT_TIMEOUT_SEC);

	while (1) {
		vTaskDelay(pdMS_TO_TICKS(1000));
		/* restart watchdog timer */
		mtk_os_hal_wdt_restart();

		/* This is normal boot up, try test case #1 & #2 */
		if (rst_sta == OS_WDT_NONE_RST) {
			printf("\nTest case 1: Sleep for 1 second, then restart watchdog timer.\n"
				"Expected result: watchdog should not timeout.\n");
			test_case_1_counter = WDT_TEST_CASE_1_COUNTER;
			while (test_case_1_counter--) {
				printf("\tdelay 1 second ... ");
				vTaskDelay(pdMS_TO_TICKS(1000));

				printf("restart watchdog timer\n");
				mtk_os_hal_wdt_restart();
			}
			printf("Test Case 1: Finished\n");

			printf("\nTest case 2: Sleep for 11 second.\n"
				"Expected result: watchdog should be timeout in %d seconds.\n", WDT_TIMEOUT_SEC);
			printf("\tdelay 11 seconds ... ");
			vTaskDelay(pdMS_TO_TICKS(11000));
			printf("\tdelay 11 seconds finished\n");
		}

		/* This is WDT HW Reset boot up, try test case #3 */
		if (rst_sta == OS_WDT_HW_RST) {
			printf("\nTest case 3: Sleep for 8 second, then trigger WDT SW Reset.\n"
				"Expected result: watchdog should not timeout, then trigger WDT SW Reset.\n");
			printf("\tdelay 8 seconds ... ");
			vTaskDelay(pdMS_TO_TICKS(8000));
			printf("\ttrigger WDT SW Reset\n");
			mtk_os_hal_wdt_sw_reset();
		}

		/* This is WDT SW Reset boot up */
		if (rst_sta == OS_WDT_SW_RST) {
			printf("\nTest case 1~3 finished. Press RESET button to restart again.\n");
			while(1){
				vTaskDelay(pdMS_TO_TICKS(1));
				mtk_os_hal_wdt_restart();
			}
		}
	}
}

_Noreturn void RTCoreMain(void)
{
	/* Setup Vector Table */
	NVIC_SetupVectorTable();

	/* Init UART */
	mtk_os_hal_uart_ctlr_init(uart_port_num);
	printf("\n\n=================================\n");
	printf("FreeRTOS WatchDog Timer(WDT) Demo\n");

	/* Create WDT Task */
	xTaskCreate(wdt_task, "WDT Task", APP_STACK_SIZE_BYTES, NULL, 5, NULL);
	vTaskStartScheduler();

	for (;;)
		__asm__("wfi");
}
