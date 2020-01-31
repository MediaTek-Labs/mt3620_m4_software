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
#include "os_hal_i2c.h"

/*
Additional Note:
This sample code uses ISU1 as I2C Master and ISU2 as I2C Slave for the I2C loopback test.
ISU1(SDA/SDL) should be connected to ISU2(SDA/SDL).
Note, This sample code works only on Seeed development board, NOT on AVNET development board.
Since ISU2 of AVNET development board has an additional sensor connected, if run
this sample code on AVNET development board, some I2C transport error might be happening.
One possible work around for AVNET development board is to change I2C_MAX_LEN from 64 to 8.
*/

/******************************************************************************/
/* Configurations */
/******************************************************************************/
static const uint8_t uart_port_num = OS_HAL_UART_ISU0;
static const uint8_t i2c_master_speed = I2C_SCL_1000kHz;
static const uint8_t i2c_master_port_num = OS_HAL_I2C_ISU1;
static const uint8_t i2c_slave_port_num = OS_HAL_I2C_ISU2;
static const uint8_t i2c_slave_addr = 0x20;

#define I2C_MIN_LEN 1
#define I2C_MAX_LEN 64	// For AVNET development board, please change to 8.
#define I2C_SLAVE_TIMEOUT 10000	// 10000ms

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
void i2c_master_task(void* pParameters)
{
	uint8_t write_buf[I2C_MAX_LEN] = {0};
	uint8_t read_buf[I2C_MAX_LEN] = {0};
	int i, length, ret = 0;

	printf("[I2C Demo]I2C Master Task Started. (ISU%d)\n", i2c_master_port_num);
	mtk_os_hal_i2c_speed_init(i2c_master_port_num, i2c_master_speed);
	while(1) {
i2c_master_restart:
		vTaskDelay(pdMS_TO_TICKS(1000));
		// I2C PIO Test Procedure:
		// 1. I2C Master write 1 Byte (value=X) to slave. 
		// 2. I2C Master write X Byte to Slave.
		// 3. I2C Slave read X Byte from Master.
		// 4. I2C Slave write back the received X Byte to Master.
		// 5. I2C Master read X Byte from Slave and compare the data.
		// 6. Loop Step 1~5 for X = I2C_MIN_LEN ~ I2C_MAX_LEN.
		printf("\n[I2C Demo]I2C Master/Slave loopback test for %dByte to %dByte... Start\n", I2C_MIN_LEN, I2C_MAX_LEN);
		for (length=I2C_MIN_LEN ; length<=I2C_MAX_LEN ; length++) {
			vTaskDelay(pdMS_TO_TICKS(1));

			write_buf[0] = length;
			ret = mtk_os_hal_i2c_write(i2c_master_port_num, i2c_slave_addr, write_buf, 1);
			if (ret) {
				printf("[I2C Demo]I2C_Master write %dB to I2C_Slave fail, ret:%d.\n\n", length, ret);
				goto i2c_master_restart;
			}
			vTaskDelay(pdMS_TO_TICKS(1));

			// Reset buffer
			memset(read_buf, 0, sizeof(read_buf));
			memset(write_buf, 0, sizeof(write_buf));
			for (i=0 ; i<length ; i++) {
				write_buf[i] = i;
			}

			// Write something to slave, this is blocking(Synchronous) function call.
			// It will be blocked until the remote device receive the data,
			// or until transfer error or 2000ms timeout.
			ret = mtk_os_hal_i2c_write(i2c_master_port_num, i2c_slave_addr, write_buf, length);
			if (ret) {
				printf("[I2C Demo]I2C_Master write %dB to I2C_Slave fail, ret:%d.\n\n", length, ret);
				goto i2c_master_restart;
			} else {
				// Read something from slave, this is blocking(Synchronous) function call.
				// It will be blocked until receive data from remote device,
				// or until transfer error or 2000ms timeout.
				vTaskDelay(pdMS_TO_TICKS(1));
				ret = mtk_os_hal_i2c_read(i2c_master_port_num, i2c_slave_addr, read_buf, length);
				if (ret) {
					printf("[I2C Demo]I2C_Master read %dB from I2C_Slave fail, ret:%d.\n\n", length, ret);
					goto i2c_master_restart;
				} else {
					for (i=0 ; i<length ; i++) {
						if (write_buf[i] != read_buf[i]) {
							printf("[I2C Demo]I2C_Master received data incorrect! (len=%dB)\n", length);
							printf("[I2C Demo]    Sent: 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x\n",
									write_buf[0], write_buf[1], write_buf[2], write_buf[3],
									write_buf[4], write_buf[5], write_buf[6], write_buf[7]);
							printf("[I2C Demo]    Recv: 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x\n\n",
									read_buf[0], read_buf[1], read_buf[2], read_buf[3],
									read_buf[4], read_buf[5], read_buf[6], read_buf[7]);
							goto i2c_master_restart;
						}
					}
					if (i == length) {
						printf("[I2C Demo]I2C Master/Slave %dB loopback test PASS\n", length);
					}
				}
			}
		}
		printf("[I2C Demo]I2C Master/Slave loopback test for %dByte to %dByte... Finish\n", I2C_MIN_LEN, I2C_MAX_LEN);
	}
}

void i2c_slave_task(void* pParameters)
{
	uint8_t read_buf[I2C_MAX_LEN] = {0};
	int length, ret = 0;

	printf("[I2C Demo]I2C Slave Task Started. (ISU%d)(Addr:0x%x)\n", i2c_slave_port_num, i2c_slave_addr);
	mtk_os_hal_i2c_set_slave_addr(i2c_slave_port_num, i2c_slave_addr);
	while (1) {
i2c_slave_restart:
		vTaskDelay(pdMS_TO_TICKS(1));

		// reset buffer
		memset(read_buf, 0, sizeof(read_buf));

		// Rx something from master, this is blocking(Synchronous) function call.
		// It will be blocked until receive data from remote device,
		// or until transfer error or 2000ms timeout.
		ret = mtk_os_hal_i2c_slave_rx(i2c_slave_port_num, read_buf, 1, I2C_SLAVE_TIMEOUT);
		if (ret < 0) {
			printf("[I2C Demo]I2C_Slave RX 1B fail, ret:%d.\n", ret);
			goto i2c_slave_restart;
		} else {
			length = read_buf[0];

			// Rx something from master, this is blocking(Synchronous) function call.
			// It will be blocked until receive data from remote device,
			// or until transfer error or 2000ms timeout.
			ret = mtk_os_hal_i2c_slave_rx(i2c_slave_port_num, read_buf, length, I2C_SLAVE_TIMEOUT);
			if (ret < 0) {
				printf("[I2C Demo]I2C_Slave RX %dB fail, ret:%d.\n", length, ret);
				goto i2c_slave_restart;
			} else {
				// Tx the read_buf back to master, this is blocking(Synchronous) function call.
				// It will be blocked until the remote device receive the data,
				// or until transfer error or 2000ms timeout.
				ret = mtk_os_hal_i2c_slave_tx(i2c_slave_port_num, read_buf, length, I2C_SLAVE_TIMEOUT);
				if (ret < 0) {
					printf("[I2C Demo]I2C_Slave TX %dB fail, ret:%d.\n", length, ret);
					goto i2c_slave_restart;
				}
			}
		}
	}
}

_Noreturn void RTCoreMain(void)
{
	// Setup Vector Table
	NVIC_SetupVectorTable();

	// Init UART
	mtk_os_hal_uart_ctlr_init(uart_port_num);
	printf("\nFreeRTOS I2C Demo\n");

	// Init I2C Master/Slave
	mtk_os_hal_i2c_ctrl_init(i2c_master_port_num);
	mtk_os_hal_i2c_ctrl_init(i2c_slave_port_num);

	// Create I2C Master/Slave Task
	xTaskCreate(i2c_master_task, "I2C Master Task", APP_STACK_SIZE_BYTES, NULL, 4, NULL);
	xTaskCreate(i2c_slave_task, "I2C Slave Task", APP_STACK_SIZE_BYTES, NULL, 7, NULL);

	vTaskStartScheduler();
	for (;;) {
		__asm__("wfi");
	}
}

