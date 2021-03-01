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
#include "os_hal_spim.h"

/****************************************************************************/
/* Configurations */
/****************************************************************************/
static const uint8_t uart_port_num = OS_HAL_UART_ISU0;

static uint8_t spi_master_port_num = OS_HAL_SPIM_ISU1;
static uint32_t spi_master_speed = 100; /* 100KHz */

#define SPIM_CLOCK_POLARITY SPI_CPOL_0
#define SPIM_CLOCK_PHASE SPI_CPHA_0
#define SPIM_RX_MLSB SPI_MSB
#define SPIM_TX_MSLB SPI_MSB
#define SPIM_FULL_DUPLEX_MIN_LEN 1
#define SPIM_FULL_DUPLEX_MAX_LEN 16

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
/* Global Variables */
/****************************************************************************/
static struct mtk_spi_config spi_default_config = {
	.cpol = SPIM_CLOCK_POLARITY,
	.cpha = SPIM_CLOCK_PHASE,
	.rx_mlsb = SPIM_RX_MLSB,
	.tx_mlsb = SPIM_TX_MSLB,
	.slave_sel = SPI_SELECT_DEVICE_0,
	.cs_polar = SPI_CS_POLARITY_LOW,
};
static uint8_t *spim_tx_buf;
static uint8_t *spim_rx_buf;
static volatile int g_async_done_flag;

/****************************************************************************/
/* Functions */
/****************************************************************************/
static int spi_xfer_complete(void *context)
{
	g_async_done_flag = 1;
	return 0;
}

static int spi_loopback_check(struct mtk_spi_transfer *trans)
{
	int i, value, err = 0;

	for (i = 0; i < trans->len; i++) {
		value = *((u8 *) trans->tx_buf + i);
		if (value != *((char *) trans->rx_buf + i))
			err++;
	}

	if (err)
		return -1;
	else
		return 0;
}

static int spi_async_transfer_test(int bus_num, int use_dma,
				   int length, int khz)
{
	int ret = 0, i;
	struct mtk_spi_transfer xfer;

	memset(&xfer, 0, sizeof(xfer));

	xfer.tx_buf = spim_tx_buf;
	xfer.rx_buf = spim_rx_buf;
	xfer.use_dma = use_dma;
	xfer.speed_khz = khz;
	xfer.len = length;
	/* Full-Duplex must send opcode, the opcode_len should be 1~4bytes */
	xfer.opcode = 0x5a;
	xfer.opcode_len = 1;

	for (i = 0; i < xfer.len; i++)
		*((char *)xfer.tx_buf + i) = 0x11 + i;

	for (i = 0; i < xfer.len; i++)
		*((char *)xfer.rx_buf + i) = 0xcc;

	g_async_done_flag = 0;
	ret = mtk_os_hal_spim_async_transfer((spim_num) bus_num,
			 &spi_default_config, &xfer, spi_xfer_complete, &xfer);
	if (ret) {
		printf("mtk_os_hal_spim_async_transfer fail\n");
		return ret;
	}

	while (g_async_done_flag == 0)
		vTaskDelay(pdMS_TO_TICKS(10));

	ret = spi_loopback_check(&xfer);
	return ret;
}

static int spi_transfer_test(int bus_num, int use_dma, int length, int khz)
{
	int ret = 0, i;
	struct mtk_spi_transfer xfer;

	memset(&xfer, 0, sizeof(xfer));

	xfer.tx_buf = spim_tx_buf;
	xfer.rx_buf = spim_rx_buf;
	xfer.use_dma = use_dma;
	xfer.speed_khz = khz;
	xfer.len = length;
	xfer.opcode = 0x5a;
	xfer.opcode_len = 1;

	for (i = 0; i < xfer.len; i++)
		*((char *)xfer.tx_buf + i) = 0x11 + i;

	for (i = 0; i < xfer.len; i++)
		*((char *)xfer.rx_buf + i) = 0xcc;

	ret = mtk_os_hal_spim_transfer((spim_num) bus_num,
				 &spi_default_config, &xfer);
	if (ret) {
		printf("mtk_os_hal_spim_transfer failed\n");
		return ret;
	}

	ret = spi_loopback_check(&xfer);
	return ret;
}

static void spim_task(void *pParameters)
{
	uint32_t counter = 0;
	uint8_t i = 0;
	uint8_t err = 0;

	printf("SPI master sync/async transfer test with legth %d ~ %d.\n",
			SPIM_FULL_DUPLEX_MIN_LEN, SPIM_FULL_DUPLEX_MAX_LEN);

	spim_tx_buf = pvPortMalloc(SPIM_FULL_DUPLEX_MAX_LEN);
	spim_rx_buf = pvPortMalloc(SPIM_FULL_DUPLEX_MAX_LEN);
	if (!spim_tx_buf || !spim_rx_buf) {
		printf("spim buf malloc fail.\n");
		return;
	}
	while (1) {
		err = 0;
		vTaskDelay(pdMS_TO_TICKS(1000));

		for (i = SPIM_FULL_DUPLEX_MIN_LEN;
		     i <= SPIM_FULL_DUPLEX_MAX_LEN;
		     i++) {
			/* FIFO Mode: Synchronous loopback test. */
			if (spi_transfer_test(spi_master_port_num, 0, i,
						spi_master_speed)) {
				printf("FIFO: spi_transfer_test error! (length=%d)\n",
					i);
				err++;
			}
			/* FIFO Mode: Asynchronous loopback test. */
			if (spi_async_transfer_test(spi_master_port_num, 0, i,
							spi_master_speed)) {
				printf("FIFO: spi_async_transfer_test error! (length=%d)\n",
					i);
				err++;
			}

			/* DMA Mode: Synchronous loopback test. */
			if (spi_transfer_test(spi_master_port_num, 1, i,
						spi_master_speed)) {
				printf("DMA: spi_transfer_test error! (length=%d)\n",
					i);
				err++;
			}
			/* DMA Mode: Asynchronous loopback test. */
			if (spi_async_transfer_test(spi_master_port_num, 1, i,
							spi_master_speed)) {
				printf("DMA: spi_async_transfer_test error! (length=%d)\n",
					i);
				err++;
			}
		}

		if (err) {
			printf("Test iteration[%ld] result: FAILED\n",
				counter++);
			goto exit;
		} else {
			printf("Test iteration[%ld] result: PASSED\n",
				counter++);
		}
	}

exit:
	printf("SPI master sync/async transfer test fail.\n");
}

_Noreturn void RTCoreMain(void)
{
	/* Setup Vector Table */
	NVIC_SetupVectorTable();

	/* Init UART */
	mtk_os_hal_uart_ctlr_init(uart_port_num);
	printf("\nFreeRTOS SPIM demo\n");

	/* Init SPIM */
	mtk_os_hal_spim_ctlr_init(spi_master_port_num);

	/* Create SPIM Task */
	xTaskCreate(spim_task, "SPIM Task",
		    APP_STACK_SIZE_BYTES, NULL, 4, NULL);

	vTaskStartScheduler();
	for (;;)
		__asm__("wfi");
}

