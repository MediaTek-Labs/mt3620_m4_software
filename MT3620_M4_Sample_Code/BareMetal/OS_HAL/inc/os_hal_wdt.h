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

#ifndef __OS_HAL_WDT_H__
#define __OS_HAL_WDT_H__

#include <mhal_wdt.h>

struct os_wdt_int {
	void (*wdt_cb_hdl)(void *);
	void *wdt_cb_data;
};

enum os_wdt_rst_sta {
	OS_WDT_NONE_RST = 0,
	OS_WDT_SW_RST = 1,
	OS_WDT_HW_RST = 2,
};

/**
 * @brief  Enable WDT.
 *  @param
 *	None
 *  @return
 *	None
 */
void mtk_os_hal_wdt_enable(void);

/**
 * @brief  Disable WDT.
 *  @param
 *	None
 *  @return
 *	None
 */
void mtk_os_hal_wdt_disable(void);

/**
 * @brief  Set wdt timeout value.
 *  @param [in] sec: wdt timeout value(unit: second)
 *  @return
 *	None
 *  @note
 *	It should be less then 60 second.
 */
int mtk_os_hal_wdt_set_timeout(unsigned int sec);

/**
 * @brief  Restart wdt counter.
 *  @param
 *	None
 *  @return
 *	None
 */
void mtk_os_hal_wdt_restart(void);

/**
 * @brief  Immediately reset core (or chip) by wdt swrst.
 *  @param
 *	None
 *  @return
 *	None
 */
void mtk_os_hal_wdt_sw_reset(void);

/**
 * @brief  Immediately reset core (or chip) by wdt timeout.
 *  @param
 *	None
 *  @return
 *	None
 *  @note
 *	None
 *  @warning
 *	None
 */
void mtk_os_hal_wdt_hw_reset(void);

/**
 * @brief  Get wdt reset status value.
 *  @param
 *	None
 *  @return
 *	The value of wdt reset status as type as enum os_wdt_rst_sta.
 */
enum os_wdt_rst_sta mtk_os_hal_wdt_get_reset_status(void);

/**
 * @brief  Config WDT mode.
 *  @param [in] irq: trigger irq(= 1), or trigger reset(= 0)
 *  @return
 *	None
 */
void mtk_os_hal_wdt_config(unsigned char irq);

/**
 * @brief  Register user interrupt handle for wdt.
 *  @param [in] wdt_int: a pointer of struct os_wdt_int to register user
 *		interrupt handle and callback data.
 *  @return
 *	None
 *  @note
 *	The default interrupt will be register if wdt_int is NULL.
 */
void mtk_os_hal_wdt_register_irq(struct os_wdt_int *wdt_int);

/**
 * @brief  Init WDT device, disable WDT.
 *  @param
 *	None
 *  @return
 *	None
 *  @note
 *	Internal assurance that it will only be executed only one time
 * effectively.
 */
void mtk_os_hal_wdt_init(void);

#endif /* __OS_HAL_WDT_H__ */
