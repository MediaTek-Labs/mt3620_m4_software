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

#include "arducam_driver.h"
#include "ov2640_regs.h"

#define VERBOSE_LOG 0

/* For Arducam SPI */
static spim_num spi_master_port_num_arducam;
static uint32_t spi_master_speed_khz;
static uint8_t spi_master_buf_max_size;

/* For Arducam I2c */
static i2c_num i2c_master_port_num_arducam;
static enum i2c_speed_kHz i2c_master_speed_arducam;
static uint8_t i2c_arducam_write_addr;

/* SPI variables */
static struct mtk_spi_config *spi_default_config;
struct mtk_spi_transfer spim_xfer;
static uint8_t *spim_tx_buf;
static uint8_t *spim_rx_buf;

#if VERBOSE_LOG
/* System tick in ms */
extern volatile uint32_t sys_tick_in_ms;
static uint32_t sys_tick_tmp;
#endif

/******************************************************************************/
/* Functions */
/******************************************************************************/
#if VERBOSE_LOG
void measure_time_begin(void)
{
	sys_tick_tmp = sys_tick_in_ms;
}

uint32_t measure_time_end(void)
{
	return sys_tick_in_ms - sys_tick_tmp;
}
#endif

int arducam_i2c_read_reg(uint8_t reg_addr, uint8_t *reg_data)
{
	int ret;
	uint8_t write_buf[1] = {0};
	uint8_t read_buf[1] = {0};

	write_buf[0] = reg_addr;
	ret = mtk_os_hal_i2c_write_read(i2c_master_port_num_arducam, i2c_arducam_write_addr, write_buf, read_buf, 1, 1);
	if (ret == 0)
		*reg_data = read_buf[0];

	return ret;
}

int arducam_i2c_write_reg(uint8_t reg_addr, uint8_t reg_data)
{
	uint8_t write_buf[2] = {0};

	write_buf[0] = reg_addr;
	write_buf[1] = reg_data;
	return mtk_os_hal_i2c_write(i2c_master_port_num_arducam, i2c_arducam_write_addr, write_buf, 2);
}

int arducam_i2c_write_regs(const struct sensor_reg reglist[])
{
	uint8_t reg_addr = 0;
	uint8_t reg_val = 0;
	const struct sensor_reg *next = reglist;

	if (reglist == NULL)
		return -1;

	reg_addr = next->reg;
	reg_val = next->val;
	while ((reg_addr != 0xff) | (reg_val != 0xff)) {
		arducam_i2c_write_reg(reg_addr, reg_val);
		next++;
		reg_addr = next->reg;
		reg_val = next->val;
	}
	return 0;
}

int arducam_spi_read_burst(uint8_t reg_addr, uint8_t len)
{
	int ret = 0;

	if (len > spi_master_buf_max_size)
		return -1;

	memset(spim_rx_buf, 0, spi_master_buf_max_size);
	spim_xfer.tx_buf = NULL;
	spim_xfer.rx_buf = spim_rx_buf;
	spim_xfer.opcode = reg_addr;
	spim_xfer.opcode_len = 1;
	spim_xfer.len = len;

	ret = mtk_os_hal_spim_transfer((spim_num) spi_master_port_num_arducam, spi_default_config, &spim_xfer);
	if (ret)
		printf("mtk_os_hal_spim_transfer failed\n");

	return ret;
}

int arducam_spi_read_reg(uint8_t reg_addr, uint8_t *reg_data)
{
	int ret = 0;

	memset(spim_rx_buf, 0, spi_master_buf_max_size);
	spim_xfer.tx_buf = NULL;
	spim_xfer.rx_buf = spim_rx_buf;
	spim_xfer.opcode = reg_addr;
	spim_xfer.opcode_len = 1;
	spim_xfer.len = 1;

	ret = mtk_os_hal_spim_transfer((spim_num) spi_master_port_num_arducam, spi_default_config, &spim_xfer);
	if (ret)
		printf("mtk_os_hal_spim_transfer failed\n");
	else
		*reg_data = spim_rx_buf[0];

	return ret;
}

int arducam_spi_write_reg(uint8_t reg_addr, uint8_t reg_data)
{
	int ret = 0;

	/* Please note that the spim_xfer.len here is 2 instead of 1.
	 * The Arducam SPI requires a tiny delay between the command phase and
	 * the data phase. But MT3620 SPI_Master does not support this tiny
	 * delay. One workaround is to extend the SPI clock from 1 byte time to
	 * 2 byte time, then, Arducam could handle the data phase correctly.
	 * Or another approach is: User could configure another GPIO to control
	 * the Arducam CS pin, and execute the SPI write operation as following:
	 *     Pull CS to low.
	 *     spim_xfer.opcode = (reg_addr | 0x80);
	 *     spim_xfer.opcode_len = 1;
	 *     spim_xfer.len = 0;
	 *     mtk_os_hal_spim_transfer();
	 *     spim_xfer.opcode = (reg_data);
	 *     spim_xfer.opcode_len = 1;
	 *     spim_xfer.len = 0;
	 *     mtk_os_hal_spim_transfer();
	 *     Pull CS to high.
	 */

	memset(spim_tx_buf, 0, spi_master_buf_max_size);
	spim_xfer.tx_buf = spim_tx_buf;
	spim_xfer.rx_buf = NULL;
	spim_xfer.opcode = (reg_addr | 0x80);
	spim_xfer.opcode_len = 1;
	spim_xfer.len = 2;
	spim_tx_buf[0] = reg_data;

	ret = mtk_os_hal_spim_transfer((spim_num)spi_master_port_num_arducam, spi_default_config, &spim_xfer);
	if (ret) {
		printf("mtk_os_hal_spim_transfer failed\n");
		return ret;
	}
	return ret;
}

void arducam_change_resolution(uint8_t resolution)
{
	switch (resolution) {
	case 0:
		arducam_i2c_write_regs(OV2640_160x120_JPEG);
		break;
	case 1:
		arducam_i2c_write_regs(OV2640_176x144_JPEG);
		break;
	case 2:
		arducam_i2c_write_regs(OV2640_320x240_JPEG);
		break;
	case 3:
		arducam_i2c_write_regs(OV2640_352x288_JPEG);
		break;
	case 4:
		arducam_i2c_write_regs(OV2640_640x480_JPEG);
		break;
	case 5:
		arducam_i2c_write_regs(OV2640_800x600_JPEG);
		break;
	case 6:
		arducam_i2c_write_regs(OV2640_1024x768_JPEG);
		break;
	case 7:
		arducam_i2c_write_regs(OV2640_1280x1024_JPEG);
		break;
	case 8:
		arducam_i2c_write_regs(OV2640_1600x1200_JPEG);
		break;
	default:
		break;
	}
	vTaskDelay(pdMS_TO_TICKS(1000));
}

void arducam_capture(void)
{
	uint8_t len1, len2, len3;
	uint8_t reg_data, length_actual;
	uint32_t length;
#if VERBOSE_LOG
	printf("Capture Start\n");
	measure_time_begin();
#endif

	/* Clear FIFO */
	arducam_spi_write_reg(0x04, 0x01);

	/* Start Capture */
	arducam_spi_write_reg(0x04, 0x02);

	/* Wait unitil capture finish */
	arducam_spi_read_reg(0x41, &reg_data);
	while (!(reg_data & 0x08)) {
		vTaskDelay(pdMS_TO_TICKS(1));
		arducam_spi_read_reg(0x41, &reg_data);
	}
#if VERBOSE_LOG
	printf("Capture Done (%ldms)\n", measure_time_end());
#endif

	/* Check Length */
	arducam_spi_read_reg(0x42, &len1);
	arducam_spi_read_reg(0x43, &len2);
	arducam_spi_read_reg(0x44, &len3);
	length = ((len3 << 16) | (len2 << 8) | len1) & 0x07fffff;
#if VERBOSE_LOG
	printf("Length=%ld Byte\n", length);
#endif

	/* Print IMG command to Arducam Host UART */
	extern void arducam_host_print(char *str, uint8_t str_len);
	arducam_host_print("ACK IMG END\r\n", 13);

	/* Read image from Arducam */
#if VERBOSE_LOG
	measure_time_begin();
#endif
	while (length > spi_master_buf_max_size) {
		/* Check length */
		length_actual = (length < spi_master_buf_max_size?length : spi_master_buf_max_size);

		/* Read from Arducam */
		arducam_spi_read_burst(0x3C, length_actual);

		/* Write IMG data to Arducam host UART */
		arducam_host_print((char *)spim_rx_buf, length_actual);

		length -= length_actual;
	}
#if VERBOSE_LOG
	printf("Transmit Done (%ldms)\n", measure_time_end());
#endif

	/* Clear FIFO */
	arducam_spi_write_reg(0x04, 0x01);
}

void arducam_init_i2c(void)
{
	uint8_t reg_data = 0;

	/* Init I2C Interface */
	mtk_os_hal_i2c_ctrl_init(i2c_master_port_num_arducam);
	mtk_os_hal_i2c_speed_init(i2c_master_port_num_arducam, i2c_master_speed_arducam);

	/* Set to Mode 1 */
	arducam_i2c_write_reg(0xFF, 0x01);

	/* Check I2C Bus */
	while (1) {
		arducam_i2c_read_reg(0x0A, &reg_data);
		if (reg_data != 0x26) {
			printf("    I2C Interface Error\n");
		} else {
			printf("    I2C interface OK.\n");
			break;
		}
		vTaskDelay(pdMS_TO_TICKS(1000));
	}
}

void arducam_init_spi(void)
{
	uint8_t reg_data = 0;

	/* Init SPI Interface */
	mtk_os_hal_spim_ctlr_init(spi_master_port_num_arducam);

	spim_tx_buf = pvPortMalloc(spi_master_buf_max_size);
	spim_rx_buf = pvPortMalloc(spi_master_buf_max_size);

	if (!spim_tx_buf || !spim_rx_buf) {
		while (1) {
			printf("SPI TX/RX Malloc fail! Need check linker.ld configuration.\n");
			vTaskDelay(pdMS_TO_TICKS(1000));
		}
	}

	memset(&spim_xfer, 0, sizeof(spim_xfer));
	memset(spim_tx_buf, 0, spi_master_buf_max_size);
	memset(spim_rx_buf, 0, spi_master_buf_max_size);
	spim_xfer.tx_buf = spim_tx_buf;
	spim_xfer.rx_buf = spim_rx_buf;
	spim_xfer.use_dma = 0;
	spim_xfer.speed_khz = spi_master_speed_khz;

	/* Reset the CPLD */
	arducam_spi_write_reg(0x07, 0x80);
	vTaskDelay(pdMS_TO_TICKS(100));
	arducam_spi_write_reg(0x07, 0x00);
	vTaskDelay(pdMS_TO_TICKS(100));

	/* Check SPI Bus */
	while (1) {
		arducam_spi_write_reg(0x00, 0x55);
		arducam_spi_read_reg(0x00, &reg_data);
		if (reg_data != 0x55) {
			printf("    SPI Interface Error (0x%02X)\n", reg_data);
			vTaskDelay(pdMS_TO_TICKS(1000));
		} else {
			printf("    SPI interface OK.\n");
			break;
		}
	}
}

void arducam_init(spim_num spi_port, uint32_t spi_clock, uint8_t spi_buf_size, struct mtk_spi_config *spi_config,
			i2c_num i2c_port, enum i2c_speed_kHz i2c_clock, uint8_t i2c_addr)
{
	uint8_t reg_data = 0;
	uint16_t product_id = 0;

	printf("  Init Arducam\n");

	spi_master_port_num_arducam = spi_port;
	spi_master_speed_khz = spi_clock;
	spi_master_buf_max_size = spi_buf_size;
	spi_default_config = spi_config;
	i2c_master_port_num_arducam = i2c_port;
	i2c_master_speed_arducam = i2c_clock;
	i2c_arducam_write_addr = i2c_addr;

	/* Init SPI */
	arducam_init_spi();

	/* Init I2C */
	arducam_init_i2c();

	/* Check ArduChip Version */
	arducam_spi_read_reg(0x40, &reg_data);
	printf("    ArduChip Version: 0x%02X\n", reg_data);

	/* Check Product ID */
	arducam_i2c_read_reg(0x0A, &reg_data);
	product_id = ((reg_data<<8) & 0xFF00);
	arducam_i2c_read_reg(0x0B, &reg_data);
	product_id |= (reg_data & 0x00FF);
	printf("    Product ID: 0x%04X\n", product_id);

	/* Initiate System Reset */
	arducam_i2c_write_reg(0x12, 0x80);
	vTaskDelay(pdMS_TO_TICKS(100));

	/* Init JPEG */
	printf("    Set to JPEG_160x120\n");
	arducam_i2c_write_regs(OV2640_JPEG_INIT);
	arducam_i2c_write_regs(OV2640_YUV422);
	arducam_i2c_write_regs(OV2640_JPEG);
	arducam_i2c_write_reg(0x15, 0x00);
	arducam_i2c_write_regs(OV2640_320x240_JPEG);

	printf("  Init Arducam Done\n");
}

