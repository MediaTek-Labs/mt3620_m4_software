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

#ifndef __OS_HAL_I2C_H__
#define __OS_HAL_I2C_H__

#include "mhal_osai.h"
#include "mhal_i2c.h"

typedef enum {
	OS_HAL_I2C_ISU0,
	OS_HAL_I2C_ISU1,
	OS_HAL_I2C_ISU2,
	OS_HAL_I2C_ISU3,
	OS_HAL_I2C_ISU4,
	OS_HAL_I2C_MAX_BUS
} I2C_BUS;

int mtk_os_hal_i2c_ctrl_init(int bus_num);
int mtk_os_hal_i2c_ctrl_deinit(int bus_num);
int mtk_os_hal_i2c_speed_init(u8 bus_num, enum i2c_speed_kHz speed);
int mtk_os_hal_i2c_read(u8 bus_num, u8 device_addr, u8 *buffer, u16 len);
int mtk_os_hal_i2c_write(u8 bus_num, u8 device_addr, u8 *buffer, u16 len);
int mtk_os_hal_i2c_write_read(u8 bus_num, u8 device_addr,
			      u8 *wr_buf, u8 *rd_buf, u16 wr_len, u16 rd_len);
int mtk_os_hal_i2c_set_slave_addr(u8 bus_num, u8 slv_addr);
int mtk_os_hal_i2c_slave_tx(u8 bus_num, u8 *buffer, u16 len, u32 time_out);
int mtk_os_hal_i2c_slave_rx(u8 bus_num, u8 *buffer, u16 len, u32 time_out);
int mtk_os_hal_i2c_slave_tx_rx(u8 bus_num, u8 *wr_buf, u8 *rd_buf,
			      u16 wr_buf_size,  u16 *rd_len, u32 time_out);

#endif
