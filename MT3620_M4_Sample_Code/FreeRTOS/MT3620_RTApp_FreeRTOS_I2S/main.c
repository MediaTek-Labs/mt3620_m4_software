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
#include "os_hal_i2s.h"

/******************************************************************************/
/* Configurations */
/******************************************************************************/
static const uint8_t uart_port_num = OS_HAL_UART_ISU0;
static const uint8_t i2s_port_num = MHAL_I2S0;

/* I2S Tx buffer length is 16KB */
#define I2S_TX_BUFFER_LENGTH 0x4000
/* I2S Rx buffer length is 16KB */
#define I2S_RX_BUFFER_LENGTH 0x4000
/* I2S Tx callback will be triggered when 4KB data is transmitted */
#define I2S_TX_BUFFER_PERIOD I2S_TX_BUFFER_LENGTH >> 2
/* I2S Rx callback will be triggered when 4KB data is received */
#define I2S_RX_BUFFER_PERIOD I2S_RX_BUFFER_LENGTH >> 2

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
/* Global Variables */
/******************************************************************************/
/* i2s_ignore_first_tx_callback: To skip the first tx callback */
static unsigned char i2s_ignore_first_tx_callback;
/* i2s_loopback_test_stop: To stop loopback test when error detected */
static unsigned char i2s_loopback_test_stop;
/* i2stxbuf: I2S Tx buffer */
static unsigned int *i2stxbuf;
/* i2srxbuf: I2S Rx buffer */
static unsigned int *i2srxbuf;
/* i2s_tx_index: (0~3) Next buffer slot to transmitted I2S data*/
static unsigned int i2s_tx_index;
/* i2s_rx_index: (0~3) Next buffer slot to receive I2S data */
static unsigned int i2s_rx_index;
/* i2s_tx_counter: (0~255), I2S Tx counter, accumulated when Tx callback */
static unsigned char i2s_tx_counter;
/* i2s_tx_counter: (0~255), I2S Rx counter, accumulated when Rx callback */
static unsigned char i2s_rx_counter;
/* audio_param_default: I2S parameters */
static audio_parameter audio_param_default = {
	MHAL_I2S_TYPE_INTERNAL_LOOPBACK_MODE,	/** Protocol mode */
	MHAL_I2S_SAMPLE_RATE_48K,		/** Sample rate */
	MHAL_I2S_BITS_PER_SAMPLE_32,		/** Word length */
	MHAL_I2S_STEREO,			/** I2S channel number */
	MHAL_I2S_LINK_CHANNLE_PER_SAMPLE_2,	/** TDM channel number */
	0,					/** TDM MSB offset */
	MHAL_FN_DIS,				/** LRCK inverse */
	MHAL_FN_DIS,				/** LR swap */
	MHAL_I2S_TX_MONO_DUPLICATE_DISABLE,	/** Tx right_ch as left_ch */
	MHAL_I2S_RX_DOWN_RATE_DISABLE,		/** RX down rate*/
	NULL,					/** TX buf point */
	I2S_TX_BUFFER_LENGTH,			/** TX buf length (BYTE) */
	I2S_TX_BUFFER_PERIOD,			/** TX period length (BYTE) */
	NULL,					/** RX buf point */
	I2S_RX_BUFFER_LENGTH,			/** RX buf length (BYTE) */
	I2S_RX_BUFFER_PERIOD,			/** RX period length (BYTE) */
	NULL,					/** TX DMA callback function */
	NULL,					/** TX callback data */
	NULL,					/** TX DMA callback function */
	NULL					/** RX callback data */
};

/******************************************************************************/
/* Functions */
/******************************************************************************/
static void i2s_tx_callback(void *data)
{
	/* Note! Don't try to do heavy job in this callback function */
	/* In this sample code, Tx buffer managemene is handled in I2STask. */

	if (i2s_ignore_first_tx_callback) {
		i2s_ignore_first_tx_callback = 0;
	} else {
		i2s_tx_index++;
		if (i2s_tx_index == 4)
			i2s_tx_index = 0;
	}
}

static void i2s_rx_callback(void *data)
{
	/* Note! Don't try to do heavy job in this callback function */
	/* In this sample code, Rx buffer managemene is handled in I2STask. */

	i2s_rx_index++;
	if (i2s_rx_index == 4)
		i2s_rx_index = 0;
}

static void i2s_loopback_fill_tx_buffer(unsigned int *u4buf,
					unsigned int u4length,
					unsigned char seed)
{
	u4buf[0] = 0xcafebeef;
	memset(&u4buf[1], seed, (u4length-1)<<2);
}

static int i2s_loopback_check_rx_buffer(unsigned int *u4buf,
					unsigned int u4length,
					unsigned char seed)
{
	unsigned int i, index, cmp_value;

	index = 0;
	for (i = 0; i < u4length; i++) {
		if (u4buf[i] == 0xcafebeef) {
			index = i;
			break;
		}
	}

	if (i >= u4length) {
		printf("Can't find magic index! [%d]\n", seed);
		i2s_loopback_test_stop = 1;
		return -1;
	}

	cmp_value = seed + ((seed) << 8) + ((seed) << 16) + ((seed) << 24);
	for (i = index+1; i < u4length-index; i++) {
		if (u4buf[i] != cmp_value) {
		printf("fail rx_buffer[0x%x],target[0x%x]\n",
			u4buf[i], cmp_value);
			i2s_loopback_test_stop = 1;
			return -1;
		}
	}

	return 0;
}

static int i2s_loopback_test(i2s_no i2s_port)
{
	int result;
	unsigned int *tx_buf = NULL;
	unsigned int *rx_buf = NULL;
	unsigned int i2s_tx_index_old;
	unsigned int i2s_rx_index_old;
	static unsigned int i2s_test_counter;

	i2s_test_counter++;
	printf("[%d]I2S Loopback Start.\n", i2s_test_counter);

	/* Initialize callback function */
	audio_param_default.tx_callback_func = i2s_tx_callback;
	audio_param_default.rx_callback_func = i2s_rx_callback;

	/* Initialize Rx buffer */
	rx_buf = audio_param_default.rx_buffer_addr;
	i2s_rx_index = 0;
	i2s_rx_index_old = 0;
	i2s_rx_counter = 0;
	memset(rx_buf, 0, I2S_RX_BUFFER_LENGTH);

	/* Initialize Tx buffer */
	tx_buf = audio_param_default.tx_buffer_addr;
	i2s_tx_index = 0;
	i2s_tx_index_old = 0;
	i2s_tx_counter = 0;
	memset(tx_buf, 0, I2S_TX_BUFFER_LENGTH);

	i2s_loopback_fill_tx_buffer(tx_buf, I2S_TX_BUFFER_PERIOD>>2,
				i2s_tx_counter);
	i2s_tx_counter++;
	i2s_loopback_fill_tx_buffer(tx_buf+(I2S_TX_BUFFER_PERIOD>>2),
				I2S_TX_BUFFER_PERIOD>>2, i2s_tx_counter);
	i2s_tx_counter++;
	i2s_loopback_fill_tx_buffer(tx_buf+((I2S_TX_BUFFER_PERIOD>>2)*2),
				I2S_TX_BUFFER_PERIOD>>2, i2s_tx_counter);
	i2s_tx_counter++;
	i2s_loopback_fill_tx_buffer(tx_buf+((I2S_TX_BUFFER_PERIOD>>2)*3),
				I2S_TX_BUFFER_PERIOD>>2, i2s_tx_counter);
	i2s_tx_counter++;

	/* Configure I2S
	 * Note! I2S Tx callback will be triggered for one time. No I2S data
	 * will be transmitted before invoking mtk_os_hal_enable_i2s()
	 */
	i2s_loopback_test_stop = 0;
	i2s_ignore_first_tx_callback = 1;
	result = mtk_os_hal_config_i2s(i2s_port, &audio_param_default);
	if (result) {
		printf("mtk_os_hal_config_i2s fail : %x\n", result);
		return result;
	}

	/* Enable I2S */
	result = mtk_os_hal_enable_i2s(i2s_port);
	if (result) {
		printf("mtk_os_hal_enable_i2s fail : %x\n", result);
		return result;
	}

	/* Fill Tx Buffer, Check Rx Buffer, Check Test Stop Condition */
	while (1) {
		vTaskDelay(pdMS_TO_TICKS(1));

		/* Fill Tx Buffer
		 * i2s_tx_index = 0 means buf_offset 12KB ~ 16KB is transmitted.
		 * i2s_tx_index = 1 means buf_offset 0~4KB is transmitted.
		 * i2s_tx_index = 2 means buf_offset 4KB ~ 8KB is transmitted.
		 * i2s_tx_index = 3 means buf_offset 8KB ~ 12KB is transmitted.
		 */
		if (i2s_tx_index != i2s_tx_index_old) {
			if (i2s_tx_index == 0) {
				tx_buf = audio_param_default.tx_buffer_addr +
					((I2S_TX_BUFFER_PERIOD>>2)*3);
			} else if (i2s_tx_index == 1) {
				tx_buf = audio_param_default.tx_buffer_addr;
			} else if (i2s_tx_index == 2) {
				tx_buf = audio_param_default.tx_buffer_addr +
					(I2S_TX_BUFFER_PERIOD>>2);
			} else if (i2s_tx_index == 3) {
				tx_buf = audio_param_default.tx_buffer_addr +
					((I2S_TX_BUFFER_PERIOD >> 2) * 2);
			} else {
				printf("i2s_tx_index error! (%d)\n",
					i2s_tx_index);
				return -1;
			}
			i2s_loopback_fill_tx_buffer(tx_buf,
						I2S_TX_BUFFER_PERIOD>>2,
						i2s_tx_counter);
			i2s_tx_counter++;
			i2s_tx_index_old = i2s_tx_index;
		}

		/* Check Rx Buffer
		 * i2s_rx_index = 1 means the rx data is in buf_offset 0~4KB.
		 * i2s_rx_index = 2 means the rx data is in buf_offset 4~8KB.
		 * i2s_rx_index = 3 means the rx data is in buf_offset 8~12KB.
		 * i2s_rx_index = 0 means the rx data is in buf_offset 12~16KB.
		 */
		if (i2s_rx_index != i2s_rx_index_old) {
			if (i2s_rx_index == 0) {
				rx_buf = audio_param_default.rx_buffer_addr +
						((I2S_RX_BUFFER_PERIOD>>2)*3);
			} else if (i2s_rx_index == 1) {
				rx_buf = audio_param_default.rx_buffer_addr;
			} else if (i2s_rx_index == 2) {
				rx_buf = audio_param_default.rx_buffer_addr +
						(I2S_RX_BUFFER_PERIOD>>2);
			} else if (i2s_rx_index == 3) {
				rx_buf = audio_param_default.rx_buffer_addr +
						((I2S_RX_BUFFER_PERIOD>>2)*2);
			} else {
				printf("i2s_rx_index error!\n");
				return -1;
			}

			/* Check the received data */
			i2s_loopback_check_rx_buffer(rx_buf,
						I2S_RX_BUFFER_PERIOD>>2,
						i2s_rx_counter);
			i2s_rx_counter++;

			i2s_rx_index_old = i2s_rx_index;
		}

		/* Check test stop condition */
		if (i2s_rx_counter >= 200 || i2s_loopback_test_stop)
			break;
	}

	if (i2s_loopback_test_stop)
		printf("[%d]I2S Loopback Test Result: Failed!\n",
			i2s_test_counter);
	else
		printf("[%d]I2S Loopback Test Result: Success.\n",
			i2s_test_counter);

	result = mtk_os_hal_disable_i2s(i2s_port);
	if (result) {
		printf("[%d]mtk_os_hal_disable_i2s fail : %x\n",
			result, i2s_test_counter);
		return result;
	}
	if (i2s_loopback_test_stop)
		return -1;
	else
		return 0;
}

static void I2STask(void *pParameters)
{
	unsigned int result;

	/*cpu usage is too high after software reboot*/
	vTaskDelay(pdMS_TO_TICKS(2000));

	printf("I2S Task Started.\n");

	/* Initialize Tx buffer */
	i2stxbuf = pvPortMalloc(I2S_TX_BUFFER_LENGTH);
	if (!i2stxbuf) {
		printf("I2S TX Malloc fail!\n");
		goto exit;
	}
	audio_param_default.tx_buffer_addr = i2stxbuf;

	/* Initialize Rx buffer */
	i2srxbuf = pvPortMalloc(I2S_RX_BUFFER_LENGTH);
	if (!i2srxbuf) {
		printf("I2S RX Malloc fail!\n");
		vPortFree(i2stxbuf);
		goto exit;
	}
	audio_param_default.rx_buffer_addr = i2srxbuf;

	while (1) {
		vTaskDelay(pdMS_TO_TICKS(10));
		/* Init I2S */
		result = mtk_os_hal_request_i2s(i2s_port_num);
		if (result) {
			printf("mtk_os_hal_request_i2s fail : %d\n", result);
			goto test_failed;
		}

		/* Start I2S loopback test*/
		result = i2s_loopback_test(i2s_port_num);
		if (result) {
			printf("i2s_loopback_test fail : %d\n", result);
			goto test_failed;
		}

		/* De-Init I2S */
		result = mtk_os_hal_free_i2s(i2s_port_num);
		if (result) {
			printf("mtk_os_hal_free_i2s fail : %d\n", result);
			goto test_failed;
		}
	}
test_failed:
	vPortFree(i2stxbuf);
	vPortFree(i2srxbuf);
exit:
	printf("I2S Loopback Finished!\n\n");
	vTaskDelete(NULL);
}

_Noreturn void RTCoreMain(void)
{
	/* Setup Vector Table */
	NVIC_SetupVectorTable();

	/* Init UART */
	mtk_os_hal_uart_ctlr_init(uart_port_num);
	printf("\nFreeRTOS I2S demo\n");

	/* Create I2S Task */
	xTaskCreate(I2STask, "I2S Task", APP_STACK_SIZE_BYTES, NULL, 4, NULL);

	vTaskStartScheduler();
	for (;;)
		__asm__("wfi");
}

