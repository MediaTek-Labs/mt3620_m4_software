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
#include "os_hal_dma.h"

/******************************************************************************/
/* Configurations */
/******************************************************************************/
static const uint8_t uart_port_num = OS_HAL_UART_ISU0;

#define DMA_MAX_LEN 128
#define APP_STACK_SIZE_BYTES (1024 / 4)

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
static int done_flag;

void dma_done_callback(void *user_data)
{
	done_flag = 1;
}

static int fullsize_dma_memcpy(u8 *dst, u8 *src, u32 len,
						dma_interrupt_callback isr_cb)
{
	struct dma_setting setting;
	int ret = 0;

	done_flag = 0;
	ret = mtk_os_hal_dma_alloc_chan(DMA_M2M_CH12);
	if (ret < 0) {
		printf("allocate dma ch %d failed!\n", DMA_M2M_CH12);
		return ret;
	}

	memset(&setting, 0x0, sizeof(struct dma_setting));
	setting.count = len;
	setting.src_addr = (u32)src;
	setting.dst_addr = (u32)dst;
	setting.ctrl_mode.src_inc_en = 1;
	setting.ctrl_mode.dst_inc_en = 1;
	setting.interrupt_flag = DMA_INT_COMPLETION;

	ret = mtk_os_hal_dma_config(DMA_M2M_CH12, &setting);
	if (ret < 0) {
		printf("mtk_os_hal_dma_config ret %d!\n", ret);
		goto FAIL;
	}

	ret = mtk_os_hal_dma_register_isr(DMA_M2M_CH12, isr_cb, NULL,
		DMA_INT_COMPLETION);
	if (ret < 0) {
		printf("mtk_os_hal_dma_register_isr ret %d!\n", ret);
		goto FAIL;
	}

	ret = mtk_os_hal_dma_start(DMA_M2M_CH12);
	if (ret < 0) {
		printf("mtk_os_hal_dma_start ret %d!\n", ret);
		goto FAIL;
	}

FAIL:
	return ret;
}

void dma_task(void *pParameters)
{
	int ret;
	u8 *src_buf;
	u8 *dst_buf;

	/* The DMA Src/Dst buffer should be allocated by pvPortMalloc(). */
	src_buf = (u8 *)pvPortMalloc(DMA_MAX_LEN);
	dst_buf = (u8 *)pvPortMalloc(DMA_MAX_LEN);
	if (!src_buf || !dst_buf) {
		printf("Malloc DMA buffer failed!\n");
		return;
	}

	printf("DMA Task Started.\n");
	while (1) {
		vTaskDelay(pdMS_TO_TICKS(1000));

		/* Init Buffer */
		memset(src_buf, 0x5, DMA_MAX_LEN);
		memset(dst_buf, 0x0, DMA_MAX_LEN);

		/* Init/Start DMA */
		ret = fullsize_dma_memcpy(dst_buf, src_buf, DMA_MAX_LEN,
							dma_done_callback);
		if (ret < 0) {
			printf("mtk_os_hal_dma_stop ret %d!\n", ret);
			continue;
		}

		/* Wait for DMA complete. */
		while (!done_flag)
			vTaskDelay(pdMS_TO_TICKS(1));

		/* Check DMA Result */
		if (memcmp(src_buf, dst_buf, DMA_MAX_LEN) != 0)
			printf("DMA test fail!\n");
		else
			printf("DMA test pass!\n");

		/* De-Init DMA */
		ret = mtk_os_hal_dma_stop(DMA_M2M_CH12);
		if (ret < 0)
			printf("mtk_os_hal_dma_stop ret %d!\n", ret);

		ret = mtk_os_hal_dma_release_chan(DMA_M2M_CH12);
		if (ret < 0)
			printf("mtk_os_hal_dma_release_chan ret %d!\n", ret);
	}
}

_Noreturn void RTCoreMain(void)
{
	/* Setup Vector Table */
	NVIC_SetupVectorTable();

	/* Init UART */
	mtk_os_hal_uart_ctlr_init(uart_port_num);
	printf("\nFreeRTOS DMA Demo\n");

	/* Create DMA Task */
	xTaskCreate(dma_task, "DMA Task", APP_STACK_SIZE_BYTES, NULL, 4, NULL);

	vTaskStartScheduler();
	for (;;)
		__asm__("wfi");
}

