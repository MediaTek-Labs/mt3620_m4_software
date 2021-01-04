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

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>

#include "FreeRTOS.h"
#include "task.h"
#include "printf.h"
#include "mt3620.h"

#include "os_hal_uart.h"
#include "os_hal_adc.h"

/******************************************************************************/
/* Configurations */
/******************************************************************************/
static const uint8_t uart_port_num = OS_HAL_UART_ISU0;

#define APP_STACK_SIZE_BYTES (1024 / 4)

/* ADC channel 0: GPIO41, bit 0 in bitmap. */
/* ADC channel 1: GPIO42, bit 1 in bitmap. */
/* ADC channel 2: GPIO43, bit 2 in bitmap. */
/* ADC channel 3: GPIO44, bit 3 in bitmap. */
/* ADC channel 4: GPIO45, bit 4 in bitmap. */
/* ADC channel 5: GPIO46, bit 5 in bitmap. */
/* ADC channel 6: GPIO47, bit 6 in bitmap. */
/* ADC channel 7: GPIO48, bit 7 in bitmap. */
/* Each ADC sampling data is 4 bytes in length, and the voltage data is 12bit in length (bit_4 ~ bit_15). */
/* Formula: volts = (Sampling_Data & ADC_DATA_MASK) >> ADC_DATA_BIT_OFFSET) * 2500 / 4096 (unit: millivolts)*/

#define ADC_GPIO_INDEX				41		/* ADC0 = GPIO41 */
#define ADC_DATA_MASK				(BITS(4, 15))	/* ADC sample data mask (bit_4 ~ bit_15) */
#define ADC_DATA_BIT_OFFSET			4		/* ADC sample data bit offset */

/* The following configuration represents:
 *    For One Shot Mode:
 *        *. ADC1 and ADC2 (two channels) are configured to capture ADC sampling data.
 *    For Period Mode:
 *        *. ADC1 and ADC2 (two channels) are configured to capture ADC sampling data.
 *        *. SAMPLE_RATE=1 means there will be periodically one ADC sampling data for each channel in one second.
 *        *. PERIODIC_BUF_LENGTH=16 means the buffer could store totally 16 ADC sampling data, which is 64 bytes in
 *        *. length and is used as a circular buffer.
 *        *. The ADC driver will invoke the RTApp callback function whenever 4 ADC sampling data are captured.
*/
#define SAMPLE_RATE			1	/* Total samples for "each channel" in one second. */
#define PERIODIC_BUF_LENGTH		16	/* 16 sampling data in the buffer. (16*4=64 Bytes) */
#define PERIODIC_BUF_LENGTH_PERIOD	4	/* Trigger callback whenever 4 sampling data are captured. */
#define BIT_MAP				0x6	/* ADC1 & ADC2 */
#define CHANNEL_NUM			2	/* ADC1 & ADC2 */



#define COLOR_YELLOW        "\033[1;33m"
#define COLOR_NONE          "\033[m"

unsigned int adc_rx_cnt;
u32 *adc_rx_buf_periodic_mode;
u32 *adc_rx_buf_one_shot_mode;

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
static void mtk_adc_periodic_mode_callback(void *data)
{
	/* Important Note! Don't do memory operations or heavy jobs in this callback function. */
	/* Memory operations or heavy jobs should be handled in the adc_task. */
	/* Keep only lightweight jobs in this callback function. */
	adc_rx_cnt++;
	if (adc_rx_cnt >= 4)
		adc_rx_cnt = 0;
}

static int mtk_adc_one_shot_mode_test(void)
{
	int ret = 0;
	int adc_no = 0;
	int channel_count = 0;
	u16 voltage = 0;
	struct adc_fsm_param adc_fsm_parameter;

	printf("\nADC One Shot Mode:\n");

	ret = mtk_os_hal_adc_ctlr_init();
	if (ret) {
		printf("Func:%s, line:%d fail\r\n", __func__, __LINE__);
		return -1;
	}

	adc_fsm_parameter.pmode = ADC_PMODE_ONE_TIME;
	adc_fsm_parameter.channel_map = BIT_MAP;
	adc_fsm_parameter.fifo_mode = ADC_FIFO_DIRECT;
	adc_fsm_parameter.ier_mode = ADC_FIFO_IER_RXFULL;
	adc_fsm_parameter.vfifo_addr = adc_rx_buf_one_shot_mode;
	adc_fsm_parameter.vfifo_len = CHANNEL_NUM;
	adc_fsm_parameter.rx_callback_func = NULL;
	adc_fsm_parameter.rx_callback_data = NULL;
	adc_fsm_parameter.rx_period_len = CHANNEL_NUM;

	ret = mtk_os_hal_adc_fsm_param_set(&adc_fsm_parameter);
	if (ret) {
		printf("Func:%s, line:%d fail\r\n", __func__, __LINE__);
		return -1;
	}

	ret = mtk_os_hal_adc_trigger_one_shot_once();
	if (ret) {
		printf("Func:%s, line:%d fail\r\n", __func__, __LINE__);
		return -1;
	}

	channel_count = 0;
	for (adc_no = 0; adc_no < ADC_CHANNEL_MAX; adc_no++) {
		if ((adc_fsm_parameter.channel_map) & BIT(adc_no)) {
			voltage = (u32)((adc_rx_buf_one_shot_mode[channel_count] & ADC_DATA_MASK)
							>> ADC_DATA_BIT_OFFSET) * 2500 / 4096;
			printf("\tCH:%d(GPIO%d) sameple voltage: %dmv (%08X)\n", adc_no, ADC_GPIO_INDEX + adc_no,
				voltage, adc_rx_buf_one_shot_mode[channel_count]);
			channel_count++;
		}
	}

	ret = mtk_os_hal_adc_ctlr_deinit();
	if (ret) {
		printf("Func:%s, line:%d fail\r\n", __func__, __LINE__);
		return -1;
	}

	return 0;
}

static int mtk_adc_periodic_mode_test(void)
{
	int ret = 0, timeout_count = 0;
	unsigned int adc_rx_cnt_old = 0;
	struct adc_fsm_param adc_fsm_parameter;

	printf("\nADC Periodic Mode:\n");

	ret = mtk_os_hal_adc_ctlr_init();
	if (ret) {
		printf("Func:%s, line:%d fail\r\n", __func__, __LINE__);
		return -1;
	}

	adc_fsm_parameter.pmode = ADC_PMODE_PERIODIC;
	adc_fsm_parameter.channel_map = BIT_MAP;
	adc_fsm_parameter.sample_rate = SAMPLE_RATE;
	adc_fsm_parameter.fifo_mode = ADC_FIFO_DMA;
	adc_fsm_parameter.ier_mode = ADC_FIFO_IER_RXFULL;
	adc_fsm_parameter.vfifo_addr = adc_rx_buf_periodic_mode;
	adc_fsm_parameter.vfifo_len = PERIODIC_BUF_LENGTH;
	adc_fsm_parameter.rx_period_len = PERIODIC_BUF_LENGTH_PERIOD;
	adc_fsm_parameter.rx_callback_func = mtk_adc_periodic_mode_callback;
	adc_fsm_parameter.rx_callback_data = NULL;

	ret = mtk_os_hal_adc_fsm_param_set(&adc_fsm_parameter);
	if (ret) {
		printf("Func:%s, line:%d fail (ret=%d)\r\n", __func__, __LINE__, ret);
		return -1;
	}

	printf(COLOR_YELLOW"\t[Buf#]\t0\t1\t2\t3\t4\t5\t6\t7\t8\t9\t10\t11\t12\t13\t14\t15\n"COLOR_NONE);
	printf(COLOR_YELLOW"\t[ADC#]\tADC1\tADC2\tADC1\tADC2\tADC1\tADC2\tADC1\tADC2\tADC1\tADC2\tADC1\tADC2\tADC1\t"
				"ADC2\tADC1\tADC2\n"COLOR_NONE);

	adc_rx_cnt = 0;
	adc_rx_cnt_old = 0;
	mtk_os_hal_adc_period_start();
	while (1) {
		vTaskDelay(pdMS_TO_TICKS(1));
		if (adc_rx_cnt != adc_rx_cnt_old) {
			printf("\t[%d]\t%s%ldmv\t%ldmv\t%ldmv\t%ldmv\t%s%ldmv\t%ldmv\t%ldmv\t%ldmv\t%s%ldmv\t%ldmv\t"
				"%ldmv\t%ldmv\t%s%ldmv\t%ldmv\t%ldmv\t%ldmv%s\n",
				timeout_count,
				adc_rx_cnt == 1?COLOR_YELLOW:COLOR_NONE,
				((adc_rx_buf_periodic_mode[0] & ADC_DATA_MASK) >> ADC_DATA_BIT_OFFSET) * 2500 / 4096,
				((adc_rx_buf_periodic_mode[1] & ADC_DATA_MASK) >> ADC_DATA_BIT_OFFSET) * 2500 / 4096,
				((adc_rx_buf_periodic_mode[2] & ADC_DATA_MASK) >> ADC_DATA_BIT_OFFSET) * 2500 / 4096,
				((adc_rx_buf_periodic_mode[3] & ADC_DATA_MASK) >> ADC_DATA_BIT_OFFSET) * 2500 / 4096,
				adc_rx_cnt == 2?COLOR_YELLOW:COLOR_NONE,
				((adc_rx_buf_periodic_mode[4] & ADC_DATA_MASK) >> ADC_DATA_BIT_OFFSET) * 2500 / 4096,
				((adc_rx_buf_periodic_mode[5] & ADC_DATA_MASK) >> ADC_DATA_BIT_OFFSET) * 2500 / 4096,
				((adc_rx_buf_periodic_mode[6] & ADC_DATA_MASK) >> ADC_DATA_BIT_OFFSET) * 2500 / 4096,
				((adc_rx_buf_periodic_mode[7] & ADC_DATA_MASK) >> ADC_DATA_BIT_OFFSET) * 2500 / 4096,
				adc_rx_cnt == 3?COLOR_YELLOW:COLOR_NONE,
				((adc_rx_buf_periodic_mode[8] & ADC_DATA_MASK) >> ADC_DATA_BIT_OFFSET) * 2500 / 4096,
				((adc_rx_buf_periodic_mode[9] & ADC_DATA_MASK) >> ADC_DATA_BIT_OFFSET) * 2500 / 4096,
				((adc_rx_buf_periodic_mode[10] & ADC_DATA_MASK) >> ADC_DATA_BIT_OFFSET) * 2500 / 4096,
				((adc_rx_buf_periodic_mode[11] & ADC_DATA_MASK) >> ADC_DATA_BIT_OFFSET) * 2500 / 4096,
				adc_rx_cnt == 0?COLOR_YELLOW:COLOR_NONE,
				((adc_rx_buf_periodic_mode[12] & ADC_DATA_MASK) >> ADC_DATA_BIT_OFFSET) * 2500 / 4096,
				((adc_rx_buf_periodic_mode[13] & ADC_DATA_MASK) >> ADC_DATA_BIT_OFFSET) * 2500 / 4096,
				((adc_rx_buf_periodic_mode[14] & ADC_DATA_MASK) >> ADC_DATA_BIT_OFFSET) * 2500 / 4096,
				((adc_rx_buf_periodic_mode[15] & ADC_DATA_MASK) >> ADC_DATA_BIT_OFFSET) * 2500 / 4096,
				COLOR_NONE);
			adc_rx_cnt_old = adc_rx_cnt;
			timeout_count++;
		}

		if (timeout_count == 20)
			break;
	}

	ret = mtk_os_hal_adc_period_stop();
	if (ret)
		printf("Func:%s, line:%d fail\r\n", __func__, __LINE__);

	ret = mtk_os_hal_adc_ctlr_deinit();
	if (ret) {
		printf("Func:%s, line:%d fail\r\n", __func__, __LINE__);
		return -1;
	}

	return 0;
}


void adc_task(void *pParameters)
{
	int ret = 0;
	int count = 0;

	adc_rx_buf_periodic_mode = pvPortMalloc(sizeof(int) * PERIODIC_BUF_LENGTH);
	if (!adc_rx_buf_periodic_mode) {
		printf("ADC PERIOD MODE RX Malloc fail!\n");
		goto exit;
	}

	memset(adc_rx_buf_periodic_mode, 0, (sizeof(int) * PERIODIC_BUF_LENGTH));

	adc_rx_buf_one_shot_mode = pvPortMalloc(sizeof(int) * CHANNEL_NUM);
	if (!adc_rx_buf_one_shot_mode) {
		printf("ADC ONE SHOT MODE RX Malloc fail!\n");
		vPortFree(adc_rx_buf_periodic_mode);
		goto exit;
	}

	memset(adc_rx_buf_one_shot_mode, 0, (sizeof(int) * CHANNEL_NUM));
	printf("ADC Task started\n");

	while (1) {
		vTaskDelay(pdMS_TO_TICKS(1000));
		ret = mtk_adc_one_shot_mode_test();
		if (ret) {
			printf("mtk_adc_one_shot_mode_test fail!\r\n");
			goto err_exit;
		}

		vTaskDelay(pdMS_TO_TICKS(1000));
		ret = mtk_adc_periodic_mode_test();
		if (ret) {
			printf("mtk_adc_periodic_mode_test fail!\r\n");
			goto err_exit;
		}

		printf("adc test pass(%d)\n", ++count);
		vTaskDelay(pdMS_TO_TICKS(2000));
		printf("\n");
	}

err_exit:
	vPortFree(adc_rx_buf_periodic_mode);
	vPortFree(adc_rx_buf_one_shot_mode);
exit:
	vTaskDelete(NULL);

}

_Noreturn void RTCoreMain(void)
{
	/* Setup Vector Table */
	NVIC_SetupVectorTable();

	/* Init UART */
	mtk_os_hal_uart_ctlr_init(uart_port_num);
	printf("\nFreeRTOS ADC Demo\n");

	/* Create ADC Task */
	xTaskCreate(adc_task, "ADC Task", APP_STACK_SIZE_BYTES, NULL, 4, NULL);

	vTaskStartScheduler();
	for (;;)
		__asm__("wfi");
}

