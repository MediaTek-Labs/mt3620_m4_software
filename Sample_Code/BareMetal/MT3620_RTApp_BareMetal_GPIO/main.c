/* Copyright (c) Microsoft Corporation. All rights reserved.
   Licensed under the MIT License. */


#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>

#include "mt3620-baremetal.h"
#include "mt3620-timer.h"
#include "os_hal_gpio.h"


extern uint32_t StackTop; // &StackTop == end of TCM

static _Noreturn void DefaultExceptionHandler(void);

static bool led1RedOn = false;

static const int led1RedGpio = 9;   // Green
static const int blinkIntervalsMs[] = {125, 250, 500};
static int blinkIntervalIndex = 0;
static const int numBlinkIntervals = sizeof(blinkIntervalsMs) / sizeof(blinkIntervalsMs[0]);
static void HandleBlinkTimerIrq(void);

static const int buttonAGpio = 12;
static const int buttonPressCheckPeriodMs = 10;
static void HandleButtonTimerIrq(void);

static _Noreturn void RTCoreMain(void);

// ARM DDI0403E.d SB1.5.2-3
// From SB1.5.3, "The Vector table must be naturally aligned to a power of two whose alignment
// value is greater than or equal to (Number of Exceptions supported x 4), with a minimum alignment
// of 128 bytes.". The array is aligned in linker.ld, using the dedicated section ".vector_table".

// The exception vector table contains a stack pointer, 15 exception handlers, and an entry for
// each interrupt.
#define INTERRUPT_COUNT 100 // from datasheet
#define EXCEPTION_COUNT (16 + INTERRUPT_COUNT)
#define INT_TO_EXC(i_) (16 + (i_))
const uintptr_t ExceptionVectorTable[EXCEPTION_COUNT] __attribute__((section(".vector_table")))
__attribute__((used)) = {
	[0] = (uintptr_t)&StackTop,					// Main Stack Pointer (MSP)
	[1] = (uintptr_t)RTCoreMain,				// Reset
	[2] = (uintptr_t)DefaultExceptionHandler,	// NMI
	[3] = (uintptr_t)DefaultExceptionHandler,	// HardFault
	[4] = (uintptr_t)DefaultExceptionHandler,	// MPU Fault
	[5] = (uintptr_t)DefaultExceptionHandler,	// Bus Fault
	[6] = (uintptr_t)DefaultExceptionHandler,	// Usage Fault
	[11] = (uintptr_t)DefaultExceptionHandler,	// SVCall
	[12] = (uintptr_t)DefaultExceptionHandler,	// Debug monitor
	[14] = (uintptr_t)DefaultExceptionHandler,	// PendSV
	[15] = (uintptr_t)DefaultExceptionHandler,	// SysTick

	[INT_TO_EXC(0)] = (uintptr_t)DefaultExceptionHandler,
	[INT_TO_EXC(1)] = (uintptr_t)Gpt_HandleIrq1,
	[INT_TO_EXC(2)... INT_TO_EXC(INTERRUPT_COUNT - 1)] = (uintptr_t)DefaultExceptionHandler};

static _Noreturn void DefaultExceptionHandler(void)
{
	for (;;) {
		// empty.
	}
}

static void HandleBlinkTimerIrq(void)
{
	led1RedOn = !led1RedOn;
	mtk_os_hal_gpio_set_output(led1RedGpio, led1RedOn);

	Gpt_LaunchTimerMs(TimerGpt0, blinkIntervalsMs[blinkIntervalIndex], HandleBlinkTimerIrq);
}

static void HandleButtonTimerIrq(void)
{
	// Assume initial state is high, i.e. button not pressed.
	static bool prevState = true;
	bool newState;

	unsigned int GpioValue = 0;
	mtk_os_hal_gpio_get_input(buttonAGpio, &GpioValue);
	newState = !(!GpioValue);

	if (newState != prevState) {
		bool pressed = !newState;
		if (pressed) {
			blinkIntervalIndex = (blinkIntervalIndex + 1) % numBlinkIntervals;
			Gpt_LaunchTimerMs(TimerGpt0, blinkIntervalsMs[blinkIntervalIndex], HandleBlinkTimerIrq);
		}

		prevState = newState;
	}

	Gpt_LaunchTimerMs(TimerGpt1, buttonPressCheckPeriodMs, HandleButtonTimerIrq);
}

static _Noreturn void RTCoreMain(void)
{
	// SCB->VTOR = ExceptionVectorTable
	WriteReg32(SCB_BASE, 0x08, (uint32_t)ExceptionVectorTable);

	Gpt_Init();

	// GPIO Init
	mtk_os_hal_gpio_ctlr_init();

	// Request GPIO Control
	mtk_os_hal_gpio_request(led1RedGpio);
	mtk_os_hal_gpio_request(buttonAGpio);

	// Set GPIO Direction (Optional)
	mtk_os_hal_gpio_set_direction(led1RedGpio, 1);
	mtk_os_hal_gpio_set_direction(buttonAGpio, 0);

	Gpt_LaunchTimerMs(TimerGpt0, blinkIntervalsMs[blinkIntervalIndex], HandleBlinkTimerIrq);
	Gpt_LaunchTimerMs(TimerGpt1, buttonPressCheckPeriodMs, HandleButtonTimerIrq);

	for (;;) {
		__asm__("wfi");
	}
}
