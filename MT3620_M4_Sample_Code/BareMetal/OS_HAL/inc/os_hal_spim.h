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

#ifndef __OS_HAL_SPIM_H__
#define __OS_HAL_SPIM_H__

#include "mhal_osai.h"
#include "mhal_spim.h"

/**
 * @addtogroup HAL
 * @{
 * @addtogroup SPIM
 * @{
 * This section describes the programming interfaces of the spim hal
 */

typedef enum {
	OS_HAL_SPIM_ISU0 = 0,
	OS_HAL_SPIM_ISU1 = 1,
	OS_HAL_SPIM_ISU2 = 2,
	OS_HAL_SPIM_ISU3 = 3,
	OS_HAL_SPIM_ISU4 = 4,
	OS_HAL_SPIM_ISU_MAX
} spim_num;

/**
 * @brief  Dump SPIM register value.
 *
 *  @param bus_num : SPIM ISU Port number, 0 is ISU0, 1 is ISU1, 2 is ISU2
 *
 *  @return -1 means fail.
 *  @return 0 means success.
 */
int mtk_os_hal_spim_dump_reg(spim_num bus_num);

/**
 * @brief  Init SPIM controller.
 *
 *  @param bus_num : SPIM ISU Port number, 0 is ISU0, 1 is ISU1, 2 is ISU2
 *
 *  @return -1 means fail.
 *  @return 0 means success.
 */
int mtk_os_hal_spim_ctlr_init(spim_num bus_num);

/**
 * @brief  Deinit SPIM controller.
 *
 *  @param bus_num : SPIM ISU Port number, 0 is ISU0, 1 is ISU1, 2 is ISU2
 *
 *  @return -1 means fail.
 *  @return 0 means success.
 */
int mtk_os_hal_spim_ctlr_deinit(spim_num bus_num);

/**
 * @brief  use FIFO or DMA mode to do one SPIM transfer.
 *
 *  @param bus_num : SPIM ISU Port number, 0 is ISU0, 1 is ISU1, 2 is ISU2
 *  @param  config: the HW setting
 *  @param  xfer: the data should be read/writen.
 *
 *  @return -1 means fail.
 *  @return 0 means success.
 */
int mtk_os_hal_spim_transfer(spim_num bus_num,
			     struct mtk_spi_config *config,
			     struct mtk_spi_transfer *xfer);
#endif
