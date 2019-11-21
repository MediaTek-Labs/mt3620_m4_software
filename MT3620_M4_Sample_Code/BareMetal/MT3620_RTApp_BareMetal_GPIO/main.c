/* Copyright (c) Microsoft Corporation. All rights reserved.
   Licensed under the MIT License. */

#include "mt3620.h"
#include "os_hal_gpt.h"
#include "os_hal_gpio.h"

/******************************************************************************/
/* Configurations */
/******************************************************************************/
static const uint8_t gpt_timer_button = GPT_ID_0;	// GPT_ID_0 clock speed: 1KHz or 32KHz
static const uint8_t gpt_timer_led = GPT_ID_3;		// GPT_ID_3 clock speed: 1MHz
static const uint8_t gpip_led = 9;
static const uint8_t gpio_button = 12;

static const uint32_t buttonPressCheckPeriodMs = 10;

static bool ledOn = false;
static const uint32_t blinkIntervalsMs[] = {125000, 250000, 500000};
static uint8_t blinkIntervalIndex = 0;
static const uint32_t numBlinkIntervals = sizeof(blinkIntervalsMs) / sizeof(blinkIntervalsMs[0]);

/******************************************************************************/
/* Functions */
/******************************************************************************/
void unused(void){}
static void HandleButtonTimerIrq(void* cb_data)
{
	// Assume initial state is high, i.e. button not pressed.
	static bool prevState = true;
	bool newState;

	unsigned int GpioValue = 0;
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
	ledOn = !ledOn;
	mtk_os_hal_gpio_set_output(gpip_led, ledOn);
	mtk_os_hal_gpt_stop(gpt_timer_led);
	mtk_os_hal_gpt_start(gpt_timer_led);
}

_Noreturn void RTCoreMain(void)
{
	struct os_gpt_int gpt0_int;
	struct os_gpt_int gpt1_int;
	
	// Init Vector Table
	NVIC_SetupVectorTable();

	// Init GPIO
	mtk_os_hal_gpio_ctlr_init();
	// Request GPIO Control
	mtk_os_hal_gpio_request(gpip_led);
	mtk_os_hal_gpio_request(gpio_button);
	// Set GPIO Direction
	mtk_os_hal_gpio_set_direction(gpip_led, 1);
	mtk_os_hal_gpio_set_direction(gpio_button, 0);
	
	mtk_os_hal_gpio_set_output(gpip_led, ledOn);

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

