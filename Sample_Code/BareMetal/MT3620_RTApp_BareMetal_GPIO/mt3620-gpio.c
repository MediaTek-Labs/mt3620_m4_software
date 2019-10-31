/*
 * MediaTek Inc. (C) 2019. All rights reserved.
 *
 * Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */

#include "os_hal_gpio.h"
#include "mhal_gpio.h"


#define CA7_GPIO_BASE				0x30020000
#define CM4_ADC_BASE				0x38000000
#define CM4_GPIO_PWM_GRP0_BASE	0x38010000
#define CM4_GPIO_PWM_GRP1_BASE	0x38020000
#define CM4_GPIO_PWM_GRP2_BASE	0x38030000
#define CM4_GPIO_PWM_GRP3_BASE	0x38040000
#define CM4_GPIO_PWM_GRP4_BASE	0x38050000
#define CM4_GPIO_PWM_GRP5_BASE	0x38060000

#define CM4_ISU0_I2C_BASE			0x38070000
#define CM4_ISU1_I2C_BASE			0x38080000
#define CM4_ISU2_I2C_BASE			0x38090000
#define CM4_ISU3_I2C_BASE			0x380A0000
#define CM4_ISU4_I2C_BASE			0x380B0000
#define CM4_ISU5_I2C_BASE			0x380C0000

#define CM4_I2S0_BASE				0x380D0000
#define CM4_I2S1_BASE				0x380E0000
#define PINMUX_BASE				0x30010000

static unsigned long gpio_base_addr[MHAL_GPIO_REG_BASE_MAX] = {
	CM4_GPIO_PWM_GRP0_BASE,
	CM4_GPIO_PWM_GRP1_BASE,
	CM4_GPIO_PWM_GRP2_BASE,
	CM4_GPIO_PWM_GRP3_BASE,
	CM4_GPIO_PWM_GRP4_BASE,
	CM4_GPIO_PWM_GRP5_BASE,
	CM4_ISU0_I2C_BASE,
	CM4_ISU1_I2C_BASE,
	CM4_ISU2_I2C_BASE,
	CM4_ISU3_I2C_BASE,
	CM4_ISU4_I2C_BASE,
	CM4_ISU5_I2C_BASE,
	CM4_ADC_BASE,
	CA7_GPIO_BASE,
	CM4_I2S0_BASE,
	CM4_I2S1_BASE,
	PINMUX_BASE,
};

static struct mtk_pinctrl_controller pctl;

int mtk_os_hal_gpio_request(u32 pin)
{
	return mtk_mhal_gpio_request(&pctl, pin);
}

int mtk_os_hal_gpio_free(u32 pin)
{
	return mtk_mhal_gpio_free(&pctl, pin);
}

int mtk_os_hal_gpio_get_input(u32 pin, u32 *pvalue)
{
	return mtk_mhal_gpio_get_input(&pctl, pin, pvalue);
}

int mtk_os_hal_gpio_set_output(
	u32 pin, u32 out_val)
{
	return mtk_mhal_gpio_set_output(&pctl, pin, out_val);
}

int mtk_os_hal_gpio_get_output(u32 pin, u32 *pvalue)
{
	return mtk_mhal_gpio_get_output(&pctl, pin, pvalue);
}

int mtk_os_hal_gpio_set_direction(
	u32 pin, u32 dir)
{
	return mtk_mhal_gpio_set_direction(&pctl, pin, dir);
}

int mtk_os_hal_gpio_get_direction(u32 pin, u32 *pvalue)
{
	return mtk_mhal_gpio_get_direction(&pctl, pin, pvalue);
}

int mtk_os_hal_gpio_set_pullen_pullsel(
	u32 pin, bool enable, bool isup)
{
	return mtk_mhal_gpio_set_pullen_pullsel(&pctl, pin, enable, isup);
}

int mtk_os_hal_gpio_pmx_set_mode(
	u32 pin, u32 mode)
{
	return mtk_mhal_gpio_pmx_set_mode(&pctl, pin, mode);
}

int mtk_os_hal_gpio_pmx_get_mode(u32 pin, u32 *pvalue)
{
	return mtk_mhal_gpio_pmx_get_mode(&pctl, pin, pvalue);
}

int mtk_os_hal_gpio_ctlr_init(void)
{
	int pin, reg_num;

	for (reg_num = 0; reg_num < MHAL_GPIO_REG_BASE_MAX; reg_num++)
		pctl.base[reg_num] = (void __iomem *)gpio_base_addr[reg_num];

	for (pin = 0; pin < MHAL_GPIO_MAX; pin++) {
		pctl.mtk_pins[pin].pinctrl_free = false;
	}

	pctl.gpio_mode_bits = 4;
	pctl.max_gpio_mode_per_reg = 8;
	pctl.port_shf = 2;
	pctl.port_mask = 0xf;
	pctl.pinmux_offset = 0x20;

	return 0;
}

int mtk_os_hal_gpio_ctlr_deinit(void)
{
	printf("%s release done\n", __func__);
	return 0;
}
