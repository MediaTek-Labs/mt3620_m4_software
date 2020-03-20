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

#include "FreeRTOS.h"
#include <semphr.h>
#include "task.h"
#include "printf.h"
#include "mt3620.h"
#include "ctype.h"

#include "os_hal_uart.h"
#include "os_hal_mbox.h"
#include "os_hal_mbox_shared_mem.h"

/******************************************************************************/
/* Configurations */
/******************************************************************************/
static const uint8_t uart_port_num = OS_HAL_UART_ISU1;

/* Maximum mailbox buffer len.
 *    Maximum message len: 1024B
 *                         1024 is the maximum value when HL_APP invoke send().
 *    Component UUID len : 16B
 *    Reserved data len  : 4B
*/
static const uint32_t mbox_buffer_len_max = 1048;

/* Bitmap for IRQ enable. bit_0 and bit_1 are used to communicate with HL_APP */
static const uint32_t mbox_irq_status = 0x3;

#define APP_STACK_SIZE_BYTES		(1024 / 4)

/******************************************************************************/
/* Global Variables */
/******************************************************************************/
SemaphoreHandle_t blockDeqSema;
SemaphoreHandle_t blockFifoSema;
static const u32 pay_load_start_offset = 20; /* UUID 16B, Reserved 4B */

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
/* Interrupt handler, interrupt is triggered when mailbox fifo R/W.
 *     data->event.channel: Channel_0 for A7, Channel_1 for the other M4.
 *     data->event.ne_sts: FIFO Non-Empty.interrupt
 *     data->event.nf_sts: FIFO Non-Full interrupt
 *     data->event.rd_int: Read FIFO interrupt
 *     data->event.wr_int: Write FIFO interrupt
*/
void mbox_fifo_cb(struct mtk_os_hal_mbox_cb_data *data)
{
	BaseType_t higher_priority_task_woken = pdFALSE;

	if (data->event.wr_int) {
		xSemaphoreGiveFromISR(blockFifoSema,
					&higher_priority_task_woken);
		portYIELD_FROM_ISR(higher_priority_task_woken);
	}
}

/* Interrupt handler, interrupt is triggered when mailbox shared memory R/W.
 *     data->swint.swint_channel: Channel_0 for A7, Channel_1 for the other M4.
 *     data->swint.swint_sts bit_0: A7 read data from mailbox
 *     data->swint.swint_sts bit_1: A7 write data to mailbox
*/
void mbox_swint_cb(struct mtk_os_hal_mbox_cb_data *data)
{
	BaseType_t higher_priority_task_woken = pdFALSE;

	if (data->swint.swint_sts & (1 << 1)) {
		xSemaphoreGiveFromISR(blockDeqSema,
					&higher_priority_task_woken);
		portYIELD_FROM_ISR(higher_priority_task_woken);
	}
}

void mbox_print_buf(u8 *mbox_buf, u32 mbox_data_len)
{
	u32 payload_len;
	u32 i;

	printf("Received message of %d bytes:\n", mbox_data_len);
	printf("  Component Id (16 bytes): %02X%02X%02X%02X-%02X%02X-%02X%02X-%02X%02X-%02X%02X%02X%02X%02X%02X\n",
			mbox_buf[3], mbox_buf[2], mbox_buf[1], mbox_buf[0],
			mbox_buf[5], mbox_buf[4], mbox_buf[7], mbox_buf[6],
			mbox_buf[8], mbox_buf[9], mbox_buf[10], mbox_buf[11],
			mbox_buf[12], mbox_buf[13], mbox_buf[14], mbox_buf[15]);

	/* Print reserved field as little-endian 4-byte integer. */
	printf("  Reserved (4 bytes): 0x%02X %02X %02X %02X\n",
		mbox_buf[19], mbox_buf[18], mbox_buf[17], mbox_buf[16]);

	/* Print message as hex. */
	payload_len = mbox_data_len - pay_load_start_offset;
	printf("  Payload (%d bytes as hex): ", payload_len);
	for (i = pay_load_start_offset; i < mbox_data_len; ++i)
		printf("0x%02X ", mbox_buf[i]);
	printf("\n");

	/* Print message as text. */
	printf("  Payload (%d bytes as text): ", payload_len);
	for (i = pay_load_start_offset; i < mbox_data_len; ++i)
		printf("%c", mbox_buf[i]);
	printf("\n");

	/* Convert payload, upper to lower, lower to upper.*/
	for (i = pay_load_start_offset; i < mbox_data_len; ++i) {
		if (isupper(mbox_buf[i]))
			mbox_buf[i] = tolower(mbox_buf[i]);
		else if (islower(mbox_buf[i]))
			mbox_buf[i] = toupper(mbox_buf[i]);
	}

	printf("  Send back (%d bytes as text):", payload_len);
	for (i = pay_load_start_offset; i < mbox_data_len; ++i)
		printf("%c", mbox_buf[i]);
	printf("\n\n");
}

void MBOXTask_B(void *pParameters)
{
	struct mbox_fifo_event mask;
	BufferHeader *outbound, *inbound;
	u8 *mbox_buf;
	u32 mbox_buf_len;
	u32 shared_buf_size;
	u32 allocated_buf_size;
	int result;

	printf("MBOX_B Task Started\n");

	blockDeqSema = xSemaphoreCreateBinary();
	blockFifoSema = xSemaphoreCreateBinary();

	/* Register interrupt callback */
	mask.channel = OS_HAL_MBOX_CH0;
	mask.ne_sts = 0;	/* FIFO Non-Empty interrupt */
	mask.nf_sts = 0;	/* FIFO Non-Full interrupt */
	mask.rd_int = 0;	/* Read FIFO interrupt */
	mask.wr_int = 1;	/* Write FIFO interrupt */
	mtk_os_hal_mbox_fifo_register_cb(OS_HAL_MBOX_CH0, mbox_fifo_cb, &mask);
	mtk_os_hal_mbox_sw_int_register_cb(OS_HAL_MBOX_CH0, mbox_swint_cb,
						mbox_irq_status);

	/* Get mailbox shared buffer size, defined by Azure Sphere OS. */
	if (GetIntercoreBuffers(&outbound, &inbound, &shared_buf_size) == -1) {
		printf("GetIntercoreBuffers failed\n");
		return;
	}

	/* Allocate the M4 buffer for mailbox communication */
	allocated_buf_size = shared_buf_size;
	if (allocated_buf_size > mbox_buffer_len_max)
		allocated_buf_size = mbox_buffer_len_max;
	mbox_buf = pvPortMalloc(allocated_buf_size);
	if (mbox_buf == NULL) {
		printf("pvPortMalloc failed\n");
		return;
	}

	printf("shared buf size = %d\n", shared_buf_size);
	printf("allocated buf size = %d\n", allocated_buf_size);

	while (1) {
		vTaskDelay(pdMS_TO_TICKS(10));

		/* Init buffer */
		mbox_buf_len = allocated_buf_size;
		memset(mbox_buf, 0, allocated_buf_size);

		/* Read from A7, dequeue from mailbox */
		result = DequeueData(outbound, inbound, shared_buf_size,
					mbox_buf, &mbox_buf_len);
		if (result == -1 || mbox_buf_len < pay_load_start_offset) {
			xSemaphoreTake(blockDeqSema, portMAX_DELAY);
			continue;
		}

		/* Print received message */
		mbox_print_buf(mbox_buf, mbox_buf_len);

		/* Write to A7, enqueue to mailbox */
		EnqueueData(inbound, outbound, shared_buf_size, mbox_buf,
				mbox_buf_len);
	}
}

_Noreturn void RTCoreMain(void)
{
	/* Setup Vector Table */
	NVIC_SetupVectorTable();

	/* Init UART */
	mtk_os_hal_uart_ctlr_init(uart_port_num);
	printf("\nFreeRTOS Mailbox demo\n");

	/* Open the MBOX channel of A7 <-> M4 */
	mtk_os_hal_mbox_open_channel(OS_HAL_MBOX_CH0);

	/* Create MBOX Task */
	xTaskCreate(MBOXTask_B, "MBOX_B Task", APP_STACK_SIZE_BYTES, NULL, 4,
						NULL);

	vTaskStartScheduler();
	for (;;)
		__asm__("wfi");
}
