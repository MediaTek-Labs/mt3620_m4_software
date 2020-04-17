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
#include "os_hal_i2c.h"

#include "lsm6dso_driver.h"
#include "lsm6dso_reg.h"

/******************************************************************************/
/* Configurations */
/******************************************************************************/
/* UART */
static const uint8_t uart_port_num = OS_HAL_UART_ISU0;

/* I2C */
static const uint8_t i2c_port_num = OS_HAL_I2C_ISU2;
static const uint8_t i2c_speed = I2C_SCL_50kHz;
static const uint8_t i2c_lsm6dso_addr = LSM6DSO_I2C_ADD_L>>1;
static uint8_t *i2c_tx_buf;
static uint8_t *i2c_rx_buf;

#define I2C_MAX_LEN 64
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
int32_t i2c_write(int *fD, uint8_t reg, uint8_t *buf, uint16_t len)
{
	if (buf == NULL)
		return -1;

	if (len > (I2C_MAX_LEN-1))
		return -1;

	i2c_tx_buf[0] = reg;
	if (buf && len)
		memcpy(&i2c_tx_buf[1], buf, len);
	mtk_os_hal_i2c_write(i2c_port_num, i2c_lsm6dso_addr, i2c_tx_buf, len+1);
	return 0;
}

int32_t i2c_read(int *fD, uint8_t reg, uint8_t *buf, uint16_t len)
{
	if (buf == NULL)
		return -1;

	if (len > (I2C_MAX_LEN))
		return -1;

	mtk_os_hal_i2c_write_read(i2c_port_num, i2c_lsm6dso_addr,
					&reg, i2c_rx_buf, 1, len);
	memcpy(buf, i2c_rx_buf, len);
	return 0;
}

void i2c_enum(void)
{
	uint8_t i;
	uint8_t data;

	printf("[ISU%d] Enumerate I2C Bus, Start\n", i2c_port_num);
	for (i = 0 ; i < 0x80 ; i += 2) {
		printf("[ISU%d] Address:0x%02X, ", i2c_port_num, i);
		if (mtk_os_hal_i2c_read(i2c_port_num, i, &data, 1) == 0)
			printf("Found 0x%02X\n", i);
	}
	printf("[ISU%d] Enumerate I2C Bus, Finish\n\n", i2c_port_num);
}

int i2c_init(void)
{
	/* Allocate I2C buffer */
	i2c_tx_buf = pvPortMalloc(I2C_MAX_LEN);
	i2c_rx_buf = pvPortMalloc(I2C_MAX_LEN);
	if (i2c_tx_buf == NULL || i2c_rx_buf == NULL) {
		printf("Failed to allocate I2C buffer!\n");
		return -1;
	}

	/* MT3620 I2C Init */
	mtk_os_hal_i2c_ctrl_init(i2c_port_num);
	mtk_os_hal_i2c_speed_init(i2c_port_num, i2c_speed);

	return 0;
}

void i2c_task(void *pParameters)
{
	printf("I2C Task Started. (ISU%d)\n", i2c_port_num);

	/* Enumerate I2C Bus*/
	i2c_enum();

	/* MT3620 I2C Init */
	if (i2c_init())
		return;

	/* LSM6DSO Init */
	if (lsm6dso_init(i2c_write, i2c_read))
		return;

	while (1) {
		/* Delay 1 second */
		vTaskDelay(pdMS_TO_TICKS(1000));

		/* Show Result */
		lsm6dso_show_result();
	}
}

_Noreturn void RTCoreMain(void)
{
	/* Setup Vector Table */
	NVIC_SetupVectorTable();

	/* Init UART */
	mtk_os_hal_uart_ctlr_init(uart_port_num);

	printf("\nFreeRTOS I2C LSM6DSO Demo\n");

	/* Init I2C Master/Slave */
	mtk_os_hal_i2c_ctrl_init(i2c_port_num);

	/* Create I2C Master/Slave Task */
	xTaskCreate(i2c_task, "I2C Task", APP_STACK_SIZE_BYTES, NULL, 4, NULL);

	vTaskStartScheduler();
	for (;;)
		__asm__("wfi");
}

