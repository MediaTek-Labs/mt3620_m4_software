/*
 * (C) 2005-2019 MediaTek Inc. All rights reserved.
 *
 * Copyright Statement:
 *
 * This MT3620 driver software/firmware and related documentation
 * ("MediaTek Software") are protected under relevant copyright laws.
 * The information contained herein is confidential and proprietary to
 * MediaTek Inc. ("MediaTek").
 * You may only use, reproduce, modify, or distribute (as applicable)
 * MediaTek Software if you have agreed to and been bound by this
 * Statement and the applicable license agreement with MediaTek
 * ("License Agreement") and been granted explicit permission to do so within
 * the License Agreement ("Permitted User").  If you are not a Permitted User,
 * please cease any access or use of MediaTek Software immediately.

 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT MEDIATEK SOFTWARE RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES
 * ARE PROVIDED TO RECEIVER ON AN "AS-IS" BASIS ONLY.
 * MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY
 * ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY
 * THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK SOFTWARE.
 * MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES
 * MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR STANDARD OR
 * OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO MEDIATEK SOFTWARE RELEASED HEREUNDER
 * WILL BE ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER
 * TO MEDIATEK DURING THE PRECEDING TWELVE (12) MONTHS FOR SUCH MEDIATEK
 * SOFTWARE AT ISSUE.
 */

#ifndef __OS_HAL_UART_H__
#define __OS_HAL_UART_H__

#include "mhal_uart.h"

/**
 * @addtogroup HAL
 * @{
 * @addtogroup UART
 * @{
 * This section describes the programming interfaces of the UART hal
 */

typedef enum {
	UART_PORT0 = 0,
	UART_ISU_0,
	UART_ISU_1,
	UART_ISU_2,
	UART_ISU_3,
	UART_ISU_4,
	UART_MAX_PORT
} UART_PORT;


int mtk_os_hal_uart_ctlr_init(UART_PORT bus_num);
int mtk_os_hal_uart_ctlr_deinit(UART_PORT bus_num);

void mtk_os_hal_uart_cg_gating(UART_PORT port_num);
void mtk_os_hal_uart_cg_release(UART_PORT port_num);
void mtk_os_hal_uart_sw_reset(UART_PORT port_num);
void mtk_os_hal_uart_dumpreg(UART_PORT port_num);

void mtk_os_hal_uart_set_baudrate(UART_PORT port_num, u32 baudrate);
void mtk_os_hal_uart_set_format(UART_PORT port_num,
		mhal_uart_data_len data_bit,
		mhal_uart_parity parity,
		mhal_uart_stop_bit stop_bit);
u8 mtk_os_hal_uart_get_char(UART_PORT port_num);
u8 mtk_os_hal_uart_get_char_nowait(UART_PORT port_num);
void mtk_os_hal_uart_put_char(UART_PORT port_num, u8 data);
void mtk_os_hal_uart_put_str(UART_PORT port_num, const char *msg);

void mtk_os_hal_uart_dma_enable(UART_PORT port_num);
void mtk_os_hal_uart_dma_disable(UART_PORT port_num);

void mtk_os_hal_uart_set_loopback(UART_PORT port_num, bool loopbak);
void mtk_os_hal_uart_set_hw_fc(UART_PORT port_num, u8 hw_fc);
void mtk_os_hal_uart_disable_sw_fc(UART_PORT port_num);
void mtk_os_hal_uart_set_sw_fc(UART_PORT port_num,
	u8 xon1, u8 xoff1, u8 xon2, u8 xoff2, u8 escape_data);

int mtk_os_hal_uart_clear_irq_status(UART_PORT port_num);
void mtk_os_hal_uart_set_irq(UART_PORT port_num, u8 irq_flag);

u32 mtk_os_hal_uart_dma_send_data(UART_PORT port_num,
	u8 *data, u32 len, bool vff_mode);
u32 mtk_os_hal_uart_dma_get_data(UART_PORT port_num,
	u8 *data, u32 len, bool vff_mode);

#endif
