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

#include "mt3620.h"
#include "os_hal_gpt.h"
#include "os_hal_gpio.h"

/******************************************************************************/
/* Configurations */
/******************************************************************************/
static const uint8_t gpt_timer_button = OS_HAL_GPT0;	// OS_HAL_GPT0 clock speed: 1KHz or 32KHz
static const uint8_t gpt_timer_led = OS_HAL_GPT3;		// OS_HAL_GPT3 clock speed: 1MHz
static const uint8_t gpip_led = OS_HAL_GPIO_9;			// GPIO_9 for LED Control
static const uint8_t gpio_button = OS_HAL_GPIO_12;		// GPIO_12 for Button Status Sensing

static const uint32_t buttonPressCheckPeriodMs = 10;

static bool ledOn = false;
static const uint32_t blinkIntervalsMs[] = {125000, 500000, 2000000};	// 125ms/500ms/2000ms
static uint8_t blinkIntervalIndex = 0;
static const uint32_t numBlinkIntervals = sizeof(blinkIntervalsMs) / sizeof(blinkIntervalsMs[0]);

/******************************************************************************/
/* Functions */
/******************************************************************************/
static void HandleButtonTimerIrq(void* cb_data)
{
	// Assume initial state is high, i.e. button not pressed.
	static bool prevState = true;
	bool newState;

	os_hal_gpio_data GpioValue = 0;
	mtk_os_hal_gpio_get_input(gpio_button, &GpioValue);
	newState = !(!GpioValue);

	if (newState != prevState) {
		bool pressed = !newState;
		if (pressed) {
			blinkIntervalIndex = (blinkIntervalIndex + 1) % numBlinkIntervals;
			mtk_os_hal_gpt_stop(gpt_timer_led);
			mtk_os_hal_gpt_reset_timer(gpt_timer_led, blinkIntervalsMs[blinkIntervalIndex], false);
			mtk_os_hal_gpt_start(gpt_timer_led);
		}
		prevState = newState;
	}
}

static void HandleBlinkTimerIrq(void* cb_data)
{
	int i;

	// Toggle GPIO of ISU0~ISU3
	for (i=OS_HAL_GPIO_26 ; i<=OS_HAL_GPIO_40 ; i++) {
		mtk_os_hal_gpio_set_output(i, ledOn);
	}

	ledOn = !ledOn;
	// Toggle LED Status
	mtk_os_hal_gpio_set_output(gpip_led, ledOn);
	// Restart Timer
	mtk_os_hal_gpt_stop(gpt_timer_led);
	mtk_os_hal_gpt_start(gpt_timer_led);
}

_Noreturn void RTCoreMain(void)
{
	struct os_gpt_int gpt0_int;
	struct os_gpt_int gpt1_int;
	int i;
	
	// Init Vector Table
	NVIC_SetupVectorTable();

	// Init GPIO
	mtk_os_hal_gpio_ctlr_init();
	// Init GPIO for Button as GPI
	mtk_os_hal_gpio_request(gpio_button);
	mtk_os_hal_gpio_pmx_set_mode(gpio_button, OS_HAL_MODE_6);
	mtk_os_hal_gpio_set_direction(gpio_button, OS_HAL_GPIO_DIR_INPUT);
	// Init GPIO for LED as GPO
	mtk_os_hal_gpio_request(gpip_led);
	mtk_os_hal_gpio_pmx_set_mode(gpip_led, OS_HAL_MODE_6);
	mtk_os_hal_gpio_set_direction(gpip_led, OS_HAL_GPIO_DIR_OUTPUT);
	mtk_os_hal_gpio_set_output(gpip_led, ledOn);
	// Init GPIO for ISU0~ISU2 as GPO
	for (i=OS_HAL_GPIO_26 ; i<=OS_HAL_GPIO_40 ; i++) {
		mtk_os_hal_gpio_request(i);
		mtk_os_hal_gpio_pmx_set_mode(i, OS_HAL_MODE_6);
		mtk_os_hal_gpio_set_direction(i, OS_HAL_GPIO_DIR_OUTPUT);
		mtk_os_hal_gpio_set_output(i, ledOn);
	}

	// Init GPT
	gpt0_int.gpt_cb_hdl = HandleBlinkTimerIrq;
	gpt0_int.gpt_cb_data = NULL;
	gpt1_int.gpt_cb_hdl = HandleButtonTimerIrq;
	gpt1_int.gpt_cb_data = NULL;
	mtk_os_hal_gpt_init();

	//	configure GPT0 clock speed (as 1KHz) and register GPT0 user interrupt callback handle and user data.
	mtk_os_hal_gpt_config(gpt_timer_led, false, &gpt0_int);
	//	configure GPT0 timeout value (as 125ms ) and configure it as one shot mode.
	mtk_os_hal_gpt_reset_timer(gpt_timer_led, blinkIntervalsMs[blinkIntervalIndex], false);
	//	start timer
	mtk_os_hal_gpt_start(gpt_timer_led);

	//	configure GPT1 clock speed (as 1KHz) and register GPT1 user interrupt callback handle and user data.
	mtk_os_hal_gpt_config(gpt_timer_button, false, &gpt1_int);
	//	configure GPT0 timeout value (as 10 ms) and configure it as repeat mode.
	mtk_os_hal_gpt_reset_timer(gpt_timer_button, buttonPressCheckPeriodMs, true);
	//	start timer
	mtk_os_hal_gpt_start(gpt_timer_button);

	for (;;) {
		__asm__("wfi");
	}
}

