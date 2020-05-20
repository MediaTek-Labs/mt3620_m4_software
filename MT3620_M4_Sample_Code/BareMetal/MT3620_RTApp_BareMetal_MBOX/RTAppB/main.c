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

#include "printf.h"
#include "mt3620.h"
#include "ctype.h"

#include "os_hal_uart.h"
#include "os_hal_mbox.h"
#include "os_hal_mbox_shared_mem.h"

/* Additional Note:
 *     A7 <--> M4 communication is handled by shared memory.
 *         mailbox fifo is used to transmit the address of the shared memory.
 *     M4 <--> M4 communication is handled by mailbox fifo.
 *         mailbox fifo is used to transmit data (4Byte CMD and 4Byte Data).
 */

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
#define MBOX_BUFFER_LEN_MAX 1048
static uint8_t mbox_local_buf[MBOX_BUFFER_LEN_MAX];

/* Bitmap for IRQ enable. bit_0 and bit_1 are used to communicate with HL_APP */
static const uint32_t mbox_irq_status = 0x3;

/* Bitmap for IRQ enable. bit_0 is used as M4 <--> M4 sw interrupt */
static const uint32_t mbox_irq_status_M4 = 0x1;

#define APP_STACK_SIZE_BYTES		(1024 / 4)

/******************************************************************************/
/* Global Variables */
/******************************************************************************/
volatile u8  blockDeqSema;
volatile u8  blockFifoSema;
volatile u8  blockFifoSema_M4;
volatile u8  SwIntSema_M4;
static const u32 pay_load_start_offset = 20; /* UUID 16B, Reserved 4B */

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
/* Mailbox Fifo Interrupt handler.
 * Mailbox Fifo Interrupt is triggered when mailbox fifo been R/W.
 *     data->event.channel: Channel_0 for A7, Channel_1 for the other M4.
 *     data->event.ne_sts: FIFO Non-Empty.interrupt
 *     data->event.nf_sts: FIFO Non-Full interrupt
 *     data->event.rd_int: Read FIFO interrupt
 *     data->event.wr_int: Write FIFO interrupt
*/
void mbox_fifo_cb(struct mtk_os_hal_mbox_cb_data *data)
{
	if (data->event.channel == OS_HAL_MBOX_CH0) {
		/* A7 core write data to mailbox fifo. */
		if (data->event.wr_int)
			blockFifoSema++;
	}

	if (data->event.channel == OS_HAL_MBOX_CH1) {
		/* The other M4 core write data to mailbox fifo. */
		if (data->event.wr_int)
			blockFifoSema_M4++;
	}
}

/* SW Interrupt handler.
 * SW interrupt is triggered when:
 *    1. A7 read/write the shared memory.
 *    2. The other M4 triggers SW interrupt.
 *     data->swint.swint_channel: Channel_0 for A7, Channel_1 for the other M4.
 *     Channel_0:
 *         data->swint.swint_sts bit_0: A7 read data from mailbox
 *         data->swint.swint_sts bit_1: A7 write data to mailbox
 *     Channel_1:
 *         data->swint.swint_sts bit_0: M4 sw interrupt
*/
void mbox_swint_cb(struct mtk_os_hal_mbox_cb_data *data)
{
	if (data->swint.channel == OS_HAL_MBOX_CH0) {
		if (data->swint.swint_sts & (1 << 1))
			blockDeqSema++;
	}

	if (data->swint.channel == OS_HAL_MBOX_CH1) {
		if (data->swint.swint_sts & (1 << 0))
			SwIntSema_M4++;
	}
}

void mbox_print_and_convert_buf(u8 *mbox_buf, u32 mbox_data_len)
{
	u32 payload_len;
	u32 i;

	printf("Received message of %d bytes (from A7):\n", mbox_data_len);
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

void MBOXTask_B(void)
{
	struct mbox_fifo_event mask;
	BufferHeader *outbound, *inbound;
	u32 mbox_local_buf_len;
	u32 mbox_shared_buf_size;
	int result;
	struct mbox_fifo_item item;
	u32 read_fifo_count;
	u32 swint_bit = 0;

	printf("MBOX_B Task Started\n");

	blockDeqSema = 0;
	blockFifoSema = 0;
	blockFifoSema_M4 = 0;
	SwIntSema_M4 = 0;

	/* Register interrupt callback */
	mask.channel = OS_HAL_MBOX_CH0;
	mask.ne_sts = 0;	/* FIFO Non-Empty interrupt */
	mask.nf_sts = 0;	/* FIFO Non-Full interrupt */
	mask.rd_int = 0;	/* Read FIFO interrupt */
	mask.wr_int = 1;	/* Write FIFO interrupt */
	mtk_os_hal_mbox_fifo_register_cb(OS_HAL_MBOX_CH0, mbox_fifo_cb, &mask);
	mtk_os_hal_mbox_sw_int_register_cb(OS_HAL_MBOX_CH0, mbox_swint_cb, mbox_irq_status);

	mask.channel = OS_HAL_MBOX_CH1;
	mask.ne_sts = 0;	/* FIFO Non-Empty interrupt */
	mask.nf_sts = 0;	/* FIFO Non-Full interrupt */
	mask.rd_int = 1;	/* Read FIFO interrupt */
	mask.wr_int = 1;	/* Write FIFO interrupt */
	mtk_os_hal_mbox_fifo_register_cb(OS_HAL_MBOX_CH1, mbox_fifo_cb, &mask);
	mtk_os_hal_mbox_sw_int_register_cb(OS_HAL_MBOX_CH1, mbox_swint_cb, mbox_irq_status_M4);

	/* Get mailbox shared buffer size, defined by Azure Sphere OS. */
	if (GetIntercoreBuffers(&outbound, &inbound, &mbox_shared_buf_size) == -1) {
		printf("GetIntercoreBuffers failed\n");
		return;
	}

	printf("shared buf size = %d\n", mbox_shared_buf_size);
	printf("local buf size = %d\n", MBOX_BUFFER_LEN_MAX);

	while (1) {
		osai_delay_ms(10);

		/* Handle A7 <--> M4 Communication */
		/* Init buffer */
		memset(mbox_local_buf, 0, MBOX_BUFFER_LEN_MAX);

		/* Wait for interrupt (A7 write shared memory) */
		while (blockDeqSema == 0)
			;
		blockDeqSema--;

		/* Read from A7, dequeue from mailbox */
		result = DequeueData(outbound, inbound, mbox_shared_buf_size, mbox_local_buf, &mbox_local_buf_len);
		if (result == -1 || mbox_local_buf_len < pay_load_start_offset) {
			printf("Mailbox dequeue failed!\n");
			continue;
		}

		/* Print the received message and convert it.*/
		mbox_print_and_convert_buf(mbox_local_buf, mbox_local_buf_len);

		/* Write to A7, enqueue to mailbox */
		EnqueueData(inbound, outbound, mbox_shared_buf_size, mbox_local_buf, mbox_local_buf_len);


		/* Handle M4 <--> M4 Communication */
		/* Wait for interrupt (The other M4 core write data to mailbox fifo.) */
		while (blockFifoSema_M4 == 0)
			;
		blockFifoSema_M4--;

		/* Get read fifo count */
		mtk_os_hal_mbox_ioctl(OS_HAL_MBOX_CH1, MBOX_IOGET_ACPT_FIFO_CNT, &read_fifo_count);
		if (read_fifo_count == 0) {
			printf("Read mailbox fifo count failed!\n");
			return;
		}

		/* Read from M4 mailbox */
		mtk_os_hal_mbox_fifo_read(OS_HAL_MBOX_CH1, &item, MBOX_TR_DATA_CMD);
		printf("CM4_B received from CM4_A: CMD=%08X, DATA=%08X\n", item.cmd, item.data);

		/* Write to M4 mailbox */
		item.cmd = 0xBBBBBB00 | item.data;
		printf("CM4_B Sending to CM4_A: CMD=%08X, DATA=%08X\n", item.cmd, item.data);
		mtk_os_hal_mbox_fifo_write(OS_HAL_MBOX_CH1, &item, MBOX_TR_DATA_CMD);

		/* Trigger M4 SW interrupt 0 to the other M4 core */
		mtk_os_hal_mbox_ioctl(OS_HAL_MBOX_CH1, MBOX_IOSET_SWINT_TRIG, &swint_bit);

		/* Wait for M4 SW interrupt 0 from the other M4 core */
		while (SwIntSema_M4 == 0)
			;
		SwIntSema_M4--;
		printf("CM4_B received SW interrupt from CM4_A\n\n\n\n");
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

	/* Open the MBOX channel of M4 <-> M4 */
	mtk_os_hal_mbox_open_channel(OS_HAL_MBOX_CH1);

	/* Create MBOX Task */
	MBOXTask_B();

	for (;;)
		__asm__("wfi");
}
