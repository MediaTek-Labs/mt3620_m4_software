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

#ifndef __OS_HAL_PWM_H__
#define __OS_HAL_PWM_H__

#include "mhal_osai.h"
#include "mhal_pwm.h"

/**
 * @addtogroup HAL
 * @{
 * @addtogroup PWM
 * @{
 * This section describes the programming interfaces of the pwm hal
 */

#define MAX_CHANNEL_NUM 4

/**
 * @brief  Init PWM controller.
 *
 *  @param group_num : PWM group number, 0 is group0 , 1 is group1, 2 is group2
 *  @param channel_bit_map : PWM_0 = 0x1,  PWM_1 = 0x10, PWM_2 = 0x100,
 *	PWM_3 = 0x1000
 *	For example: use PWM_0 and PWM_1
 *	channel_bit_map = PWM_0 | PWM_1
 * @return
 *	If return value is 0, it means success.\n
 *	If return value is -#EPTR , it means ctlr is NULL.\n
 *	If return value is -#ECLK , it means clock select invalid.
 */
int mtk_os_hal_pwm_ctlr_init(int group_num, u32 channel_bit_map);
/**
 * @brief  Deinit PWM controller.
 *
 *  @param group_num : PWM group number, 0 is group0 , 1 is group1, 2 is group2
 *  @param channel_bit_map : PWM_0 = 0x1,  PWM_1 = 0x10, PWM_2 = 0x100,
 *	PWM_3 = 0x1000
 *	For example: use PWM_0 and PWM_1
 *	channel_bit_map = PWM_0 | PWM_1
 *
 * @return
 *	If return value is 0, it means success.\n
 *	If return value is -#EPTR , it means ctlr is NULL.\n
 *	If return value is -#EPARAMETER , it means pwm_num is invalid.
 */
int mtk_os_hal_pwm_ctlr_deinit(int group_num, u32 channel_bit_map);
/**
 * @brief  Config freq & duty.
 *
 *  @param group_num : PWM group number, 0 is group0 , 1 is group1, 2 is group2
 *  @param pwm_num : PWM number, 0 is pwm0 , 1 is pwm1, 2 is pwm2, 3 is pwm3
 *
 * @return
 *	If return value is 0, it means success.\n
 *	If return value is -#EPTR , it means ctlr is NULL.\n
 *	If return value is -#EPARAMETER , it means pwm_num is invalid.
 */
int mtk_os_hal_pwm_config_freq_duty_normal(int group_num,
	pwm_channel pwm_num, u32  frequency, u32  duty_cycle);

/**
 * @brief  PWM feature enable.
 *
 *  @param group_num : PWM group number, 0 is group0 , 1 is group1, 2 is group2
 *  @param pwm_num : PWM number, 0 is pwm0 , 1 is pwm1, 2 is pwm2, 3 is pwm3
 *
 * @return
 *	If return value is 0, it means success.\n
 *	If return value is -#EPTR , it means ctlr is NULL.\n
 *	If return value is -#EPARAMETER , it means pwm_num is invalid.
 */
int mtk_os_hal_pwm_feature_enable(int group_num,
	pwm_channel pwm_num,
	bool global_kick_enable,
	bool io_ctrl_sel,
	bool polarity_set);

/**
 * @brief Config 2-state freq & duty.
 *
 *  @param group_num : PWM group number, 0 is group0 , 1 is group1, 2 is group2
 *  @param pwm_num : PWM number, 0 is pwm0 , 1 is pwm1, 2 is pwm2, 3 is pwm3
 *
 * @return
 *	If return value is 0, it means success.\n
 *	If return value is -#EPTR , it means ctlr is NULL.\n
 *	If return value is -#EPARAMETER , it means pwm_num is invalid.
 */
int mtk_os_hal_pwm_config_freq_duty_2_state(int group_num,
		pwm_channel pwm_num,
		struct mtk_com_pwm_data state_config);
/**
 * @brief Config 2-state stay cycle.
 *
 *  @param group_num : PWM group number, 0 is group0 , 1 is group1, 2 is group2
 *  @param pwm_num : PWM number, 0 is pwm0 , 1 is pwm1, 2 is pwm2, 3 is pwm3
 *
 * @return
 *	If return value is 0, it means success.\n
 *	If return value is -#EPTR , it means ctlr is NULL.\n
 *	If return value is -#EPARAMETER , it means pwm_num is invalid.
*/
int mtk_os_hal_pwm_config_stay_cycle_2_state(int group_num,
		pwm_channel pwm_num,
		struct mtk_com_pwm_data state_config);

/**
 * @brief  Config dpsel.
 *
 *  @param group_num : PWM group number, 0 is group0 , 1 is group1, 2 is group2
 *  @param pwm_num : PWM number, 0 is pwm0 , 1 is pwm1, 2 is pwm2, 3 is pwm3
 *
 * @return
 *	If return value is 0, it means success.\n
 *	If return value is -#EPTR , it means ctlr is NULL.\n
 *	If return value is -#EPARAMETER , it means pwm_num is invalid.
 */
int mtk_os_hal_pwm_config_dpsel(int group_num,
		pwm_channel pwm_num,
		pwm_differential_select mode);
/**
 * @brief  Start PWM controller.
 *
 *  @param group_num : PWM group number, 0 is group0 , 1 is group1, 2 is group2
 *  @param pwm_num : PWM number, 0 is pwm0 , 1 is pwm1, 2 is pwm2, 3 is pwm3
 *
 * @return
 *	If return value is 0, it means success.\n
 *	If return value is -#EPTR , it means ctlr is NULL.\n
 *	If return value is -#EPARAMETER , it means pwm_num is invalid.
 */
int mtk_os_hal_pwm_start_normal(int group_num,
		pwm_channel pwm_num);
/**
 * @brief  Stop PWM controller.
 *
 *  @param group_num : PWM group number, 0 is group0 , 1 is group1, 2 is group2
 *  @param pwm_num : PWM number, 0 is pwm0 , 1 is pwm1, 2 is pwm2, 3 is pwm3
 *
 * @return
 *	If return value is 0, it means success.\n
 *	If return value is -#EPTR , it means ctlr is NULL.\n
 *	If return value is -#EPARAMETER , it means pwm_num is invalid.
 */
int mtk_os_hal_pwm_stop_normal(int group_num,
		pwm_channel pwm_num);

#endif

