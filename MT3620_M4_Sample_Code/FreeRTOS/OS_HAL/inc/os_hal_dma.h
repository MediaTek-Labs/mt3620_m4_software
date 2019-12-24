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

#ifndef __OS_HAL_DMA_H__
#define __OS_HAL_DMA_H__

#include "mhal_osai.h"
#include "mhal_dma.h"

/**
 * @addtogroup HAL
 * @{
 * @addtogroup DMA
 * @{
 * This section describes the programing api of DMA HAL.
 */
#define DMA_SIZE_BYTE 0x00000000
#define DMA_SIZE_SHORT 0x00000001
#define DMA_SIZE_LONG 0x00000002

/** @brief options for HALF-SIZE & FULL-SIZE DMA channels */
enum dma_channel {
	DMA_ISU0_TX_CH0,   /**< HALF-SIZE DMA channel 0 as ISU0-TX */
	DMA_ISU0_RX_CH1,   /**< HALF-SIZE DMA channel 1 as ISU0-RX */
	DMA_ISU1_TX_CH2,   /**< HALF-SIZE DMA channel 2 as ISU1-TX */
	DMA_ISU1_RX_CH3,   /**< HALF-SIZE DMA channel 3 as ISU1-RX */
	DMA_ISU2_TX_CH4,   /**< HALF-SIZE DMA channel 4 as ISU2-TX */
	DMA_ISU2_RX_CH5,   /**< HALF-SIZE DMA channel 5 as ISU2-RX */
	DMA_ISU3_TX_CH6,   /**< HALF-SIZE DMA channel 6 as ISU3-TX */
	DMA_ISU3_RX_CH7,   /**< HALF-SIZE DMA channel 7 as ISU3-RX */
	DMA_ISU4_TX_CH8,   /**< HALF-SIZE DMA channel 8 as ISU4-TX */
	DMA_ISU4_RX_CH9,   /**< HALF-SIZE DMA channel 9 as ISU4-RX */
	DMA_M2M_CH12 = 12,      /**< FULL-SIZE DMA channel 12 as memory copy */
	VDMA_ISU0_TX_CH13, /**< VFF DMA channel 13 as ISU0-TX */
	VDMA_ISU0_RX_CH14, /**< VFF DMA channel 14 as ISU0-RX */
	VDMA_ISU1_TX_CH15, /**< VFF DMA channel 15 as ISU1-TX */
	VDMA_ISU1_RX_CH16, /**< VFF DMA channel 16 as ISU1-RX */
	VDMA_ISU2_TX_CH17, /**< VFF DMA channel 17 as ISU2-TX */
	VDMA_ISU2_RX_CH18, /**< VFF DMA channel 18 as ISU2-RX */
	VDMA_ISU3_TX_CH19, /**< VFF DMA channel 19 as ISU3-TX */
	VDMA_ISU3_RX_CH20, /**< VFF DMA channel 20 as ISU3-RX */
	VDMA_ISU4_TX_CH21, /**< VFF DMA channel 21 as ISU4-TX */
	VDMA_ISU4_RX_CH22, /**< VFF DMA channel 22 as ISU4-RX */
	VDMA_I2S0_TX_CH25 = 25, /**< VFF DMA channel 25 as I2S0-TX */
	VDMA_I2S0_RX_CH26, /**< VFF DMA channel 26 as I2S0-RX */
	VDMA_I2S1_TX_CH27, /**< VFF DMA channel 27 as I2S1-TX */
	VDMA_I2S1_RX_CH28, /**< VFF DMA channel 28 as I2S1-RX */
	VDMA_ADC_RX_CH29,  /**< VFF DMA channel 29 as ADC-RX */
};

enum dma_param_type {
	OS_HAL_DMA_PARAM_RLCT = 0,
	OS_HAL_DMA_PARAM_FIX_ADDR = 1,
	OS_HAL_DMA_PARAM_PROG_ADDR = 2,
	OS_HAL_DMA_PARAM_VFF_FIFO_SIZE = 3,
	OS_HAL_DMA_PARAM_VFF_FIFO_CNT = 4,
	OS_HAL_DMA_PARAM_VFF_HWPTR = 5,
	OS_HAL_DMA_PARAM_VFF_SWPTR = 6,
};

enum dma_interrupt_type {
	DMA_INT_COMPLETION = 0x1 << 0,
	DMA_INT_HALF_COMPLETION = 0x1 << 1,
	DMA_INT_VFIFO_TIMEOUT = 0x1 << 2,
	DMA_INT_VFIFO_THRESHOLD = 0x1 << 3,
};

typedef void (*dma_interrupt_callback)(void *user_data);

struct dma_wrap {
	u8 wrap_en;
	u8 wrap_side;
	u32 wrap_point;
	u32 wrap_to_addr;
};

struct dma_control_mode {
	u8 burst_type;
	u8 bw_transfer_en;
	u8 dst_inc_en;
	u8 src_inc_en;
	u8 transize;
	u8 bw_limiter;
	struct dma_wrap wrap_settings;
};

struct dma_interrupt {
	dma_interrupt_callback isr_cb;
	void *cb_data;
};

/** @brief dma_vfifo_parameter */
struct dma_vfifo {
	u32 fifo_thrsh;
	u8 alert_cmp_type;
	u32 alert_len;
	u32 fifo_size;
	u32 timeout_cnt;
};

/** @brief dma_parameter */
struct dma_setting {
	u8 interrupt_flag;
	u8 dir;
	u32 src_addr;
	u32 dst_addr;
	u32 count;
	u8 dreq;
	u8 reload_en;
	struct dma_vfifo vfifo;
	struct dma_control_mode ctrl_mode;
};

int mtk_os_hal_dma_alloc_chan(u8 chn);
int mtk_os_hal_dma_config(u8 chn, struct dma_setting *setting);
int mtk_os_hal_dma_start(u8 chn);
int mtk_os_hal_dma_stop(u8 chn);
int mtk_os_hal_dma_pause(u8 chn);
int mtk_os_hal_dma_resume(u8 chn);
int mtk_os_hal_dma_get_status(u8 chn);
int mtk_os_hal_dma_register_isr(u8 chn, dma_interrupt_callback callback,
				void *callback_data,
				enum dma_interrupt_type isr_type);
int mtk_os_hal_dma_dump_register(u8 chn);
int mtk_os_hal_dma_set_param(u8 chn, enum dma_param_type param_type,
			     u32 value);
int mtk_os_hal_dma_get_param(u8 chn, enum dma_param_type param_type);
int mtk_os_hal_dma_update_swptr(u8 chn, u32 length_byte);
int mtk_os_hal_dma_vff_read_data(u8 chn, u8 *buffer, u32 length);
int mtk_os_hal_dma_release_chan(u8 chn);
int mtk_os_hal_dma_reset(u8 chn);
int mtk_os_hal_dma_clr_dreq(u8 chn);

/**
 * @}
 * @}
 */
#endif
