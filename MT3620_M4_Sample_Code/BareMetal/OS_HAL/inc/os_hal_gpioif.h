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

#ifndef __OS_HAL_GPIOIF_H__
#define __OS_HAL_GPIOIF_H__

#include "mhal_osai.h"
#include "mhal_gpioif.h"

/**
 * @addtogroup HAL
 * @{
 * @addtogroup GPIO
 * @{
 * This section describes the programming interfaces of the GPIO hal
 */

#define MTK_GPIOIF_MAX_GRP_NUM	6

/** @brief	This defines the callback function prototype.
 * It's used for GPIOIF interrupt function handle.\n
 * User can register a callback function when using GPIOIF interrupt.\n
 * When one interrupt is triggered and a related user callback
 * is called in OS-HAL interrupt handle function which we register.\n
 * This typedef is used to describe the callback function
 * what the user wants to call.
 *
 * @param [in] user_data: An OS-HAL defined parameter provided
 * by #mtk_os_hal_gpioif_int_callback_register().
 * @sa  #mtk_mhal_gpioif_int_callback_register()
 *
 *@return  Return "0" if callback register successfully, or return "-1" if fail.
 */
typedef int (*gpioif_int_callback) (void *user_data);

/**
 * @brief  Init GPIOIF controller.
 *
 *  @return none.
 */
int mtk_os_hal_gpioif_ctlr_init(u8 group);

/**
 * @brief  Deinit GPIOIF controller.
 *
 *  @param[in] grp_num : GPIOIF group number.
 *
 *  @return none.
 */
int mtk_os_hal_gpioif_ctlr_deinit(u8 group);

int mtk_os_hal_gpioif_set_direction_mode(
	u8 group, u8 control_setting, u32 low_limit,
	u32 high_limit, u32 reset_value, u8 clock_source);

int mtk_os_hal_gpioif_set_updown_mode(
	u8 group, u8 control_setting, u32 low_limit,
	u32 high_limit, u32 reset_value, u8 clock_source);

int mtk_os_hal_gpioif_set_quadrature_mode(
	u8 group, u8 control_setting, u32 low_limit,
	u32 high_limit, u32 reset_value, u8 clock_source);

int mtk_os_hal_gpioif_set_capture_mode(
	u8 group, u8 edge_type_gpio_0, u8 edge_type_gpio_1, u8 clock_source);

int mtk_os_hal_gpioif_interrupt_control(
	u8 group, u8 enable, u8 clear, u32 bit);

int mtk_os_hal_gpioif_limit_comparator(
	u8 group, u8 sa_limit_v, u8 interrupt_limit_v);

int mtk_os_hal_gpioif_de_glitch(
	u8 group, u8 gpio, u8 enable, u32 min_p, u32 init_v);

int mtk_os_hal_gpioif_global_cr_reset(u8 group);

int mtk_os_hal_gpioif_hardware_reset(u8 group, u8 mode, u8 active_reset);

int mtk_os_hal_gpioif_interrupt_count_init(u8 group);

int mtk_os_hal_gpioif_capture_fifo_init(u8 group);

int mtk_os_hal_gpioif_dump_int(u8 group);

int mtk_os_hal_gpioif_read_gpio_event_count(u8 group, u32 *pvalue);

int mtk_os_hal_gpioif_read_reset_val(u8 group, u32 *pvalue);

int mtk_os_hal_gpioif_read_low_limit_val(u8 group, u32 *pvalue);

int mtk_os_hal_gpioif_read_high_limit_val(u8 group, u32 *pvalue);

int mtk_os_hal_gpioif_select_clock_source(u8 group, u8 clock_source);

int mtk_os_hal_gpioif_counter_clock_setting(u8 group, u8 enable);

int mtk_os_hal_gpioif_software_reset(u8 group, u8 mode);

int mtk_os_hal_gpioif_enable_event_counter(u8 group);

int mtk_os_hal_gpioif_event_counter_setting(
	u8 group, u8 control_setting, u32 low_limit, u32 high_limit,
	u32 reset_value, u8 event_cnt_mode);

int mtk_os_hal_gpioif_read_gpio_cap_fifo0_value(u8 group, u32 *pvalue);

int mtk_os_hal_gpioif_read_gpio_cap_fifo1_value(u8 group, u32 *pvalue);

int mtk_os_hal_gpioif_get_int_event_low_count(u8 group);

int mtk_os_hal_gpioif_get_int_event_high_count(u8 group);

int mtk_os_hal_gpioif_get_int_gpio2_rst_done_count(u8 group);

int mtk_os_hal_gpioif_get_int_event_over_count(u8 group);

int mtk_os_hal_gpioif_get_int_event_uf_count(u8 group);

int mtk_os_hal_gpioif_get_int_cap_f0_full_count(u8 group);

int mtk_os_hal_gpioif_get_int_cap_f1_full_count(u8 group);

int mtk_os_hal_gpioif_get_int_reset_cap_f0_full_count(u8 group);

int mtk_os_hal_gpioif_get_int_reset_cap_f1_full_count(u8 group);

int mtk_os_hal_gpioif_get_int_cap_f0_np_count(u8 group);

int mtk_os_hal_gpioif_get_int_cap_f1_np_count(u8 group);

int mtk_os_hal_gpioif_get_int_cap_f0_p_count(u8 group);

int mtk_os_hal_gpioif_get_int_cap_f1_p_count(u8 group);

int mtk_os_hal_gpioif_interrupt_bit_wise(u8 group, u32 bit, u8 enable);

int mtk_os_hal_gpioif_get_cap_fifo0_count(u8 group);

int mtk_os_hal_gpioif_get_cap_fifo1_count(u8 group);

int mtk_os_hal_gpioif_get_cap_fifo0_val(u8 group, u32 idex);

int mtk_os_hal_gpioif_get_cap_fifo1_val(u8 group, u32 idex);

/** @brief This function is used to register user's interrupt callback.
 * It's used for GPIOIF interrupt function handle.\n
 * User can register a callback function when using GPIOIF interrupt.\n
 * When one interrupt is triggered and a related user callback
 * is called in OS-HAL interrupt handle function which we register.\n
 * This typedef is used to describe the callback function
 * what the user wants to call.
 *
 * @param [in] ctlr : GPIOIF controller used with the device.
 * @param [in] callback : The callback function given by test layer.
 * which will be called before we set gpioif related setting.
 * @param [in] user_data : A parameter given by OS-HAL and will
 * be passed to user when the callback function is called.
 *
 *@return
 * Return "0" if callback register successfully.\n
 * Return -#EINVAL if ctlr or callback or user_data is NULL.
 */
int mtk_os_hal_gpioif_int_callback_register(u8 group,
	gpioif_int_callback callback, void *user_data);

/**
* @}
* @}
*/
#endif
