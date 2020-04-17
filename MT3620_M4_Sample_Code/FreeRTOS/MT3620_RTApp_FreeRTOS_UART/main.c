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

/****************************************************************************/
/* Configurations */
/****************************************************************************/
static const UART_PORT uart_port_num = OS_HAL_UART_ISU0;
static const mhal_uart_data_len uart_dat_len = UART_DATA_8_BITS;
static const mhal_uart_parity uart_parity = UART_NONE_PARITY;
static const mhal_uart_stop_bit uart_stop_bit = UART_STOP_1_BIT;
static const uint32_t uart_baudrate = 115200;
static const uint32_t uart_dma_timeout = 100; /* 100 ms */

#define APP_STACK_SIZE_BYTES		(1024 / 4)

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
static void uart_rx_task(void *pParameters)
{
	uint8_t data;

	printf("UART Rx task started, keep monitoring user input.\n");
	while (1) {
		/* Delay 10ms */
		vTaskDelay(pdMS_TO_TICKS(10));

		/* Get UART input */
		data = mtk_os_hal_uart_get_char(uart_port_num);
		printf("UART Rx: %c\n", data);
	}
}

static void uart_tx_task(void *pParameters)
{
	#define DMA_BUF_SIZE 64
	uint8_t counter = 0;
	int32_t ret = 0;
	char *dma_buf = NULL;

	dma_buf = pvPortMalloc(DMA_BUF_SIZE);
	printf("UART Tx task started, print log for every second.\n");
	while (1) {
		/* Delay 1000ms */
		vTaskDelay(pdMS_TO_TICKS(1000));

		/* print log to UART by printf, _putchar() will be invoked. */
		printf("UART Tx(printf) Counter ... %d\n", counter++);

		/* Delay 1000ms */
		vTaskDelay(pdMS_TO_TICKS(1000));

		/* print log to UART by DMA, _putchar will not be invoked. */
		memset(dma_buf, 0, DMA_BUF_SIZE);
		snprintf(dma_buf, DMA_BUF_SIZE, "\rUART Tx(DMA)    Counter ... %d\r\n", counter++);
		ret = mtk_os_hal_uart_dma_send_data(uart_port_num, (u8 *)dma_buf, DMA_BUF_SIZE, 0, uart_dma_timeout);
		if (ret < 0)
			printf("UART Tx(DMA) Error! (%ld)\n", ret);
		else if (ret != DMA_BUF_SIZE)
			printf("UART Tx(DMA) not completed! Byte transferred:%ld)\n", ret);
	}
}

_Noreturn void RTCoreMain(void)
{
	/* Setup Vector Table */
	NVIC_SetupVectorTable();

	/* Init UART */
	mtk_os_hal_uart_ctlr_init(uart_port_num);
	mtk_os_hal_uart_set_format(uart_port_num, uart_dat_len, uart_parity, uart_stop_bit);
	mtk_os_hal_uart_set_baudrate(uart_port_num, uart_baudrate);
	printf("\nFreeRTOS UART Demo\n");

	/* Create UART Tx Task */
	xTaskCreate(uart_tx_task, "UART Tx Task", APP_STACK_SIZE_BYTES, NULL, 5, NULL);

	/* Create UART Rx Task */
	xTaskCreate(uart_rx_task, "UART Rx Task", APP_STACK_SIZE_BYTES, NULL, 4, NULL);

	vTaskStartScheduler();
	for (;;)
		__asm__("wfi");
}

