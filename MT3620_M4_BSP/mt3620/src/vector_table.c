/* Copyright (c) Microsoft Corporation. All rights reserved.
   Licensed under the MIT License. */

#include <stdint.h>
#include "vector_table.h"

extern uint32_t StackTop;

static _Noreturn void DefaultExceptionHandler(void)
{
	for (;;) {
		// empty.
	}
}

void __attribute__((weak, alias("DefaultExceptionHandler"))) NMI_Handler(void);
void __attribute__((weak, alias("DefaultExceptionHandler"))) Hard_Fault_Handler(void);
void __attribute__((weak, alias("DefaultExceptionHandler"))) MPU_Fault_Handler(void);
void __attribute__((weak, alias("DefaultExceptionHandler"))) Bus_Fault_Handler(void);
void __attribute__((weak, alias("DefaultExceptionHandler"))) Usage_Fault_Handler(void);
void __attribute__((weak, alias("DefaultExceptionHandler"))) SVCall__Handler(void);
void __attribute__((weak, alias("DefaultExceptionHandler"))) Debug_Monitor_Handler(void);
void __attribute__((weak, alias("DefaultExceptionHandler"))) Pend_SV__Handler(void);
void __attribute__((weak, alias("DefaultExceptionHandler"))) Sys_Tick_Handler(void);

// ARM DDI0403E.d SB1.5.2-3
// From SB1.5.3, "The Vector table must be naturally aligned to a power of two whose alignment
// value is greater than or equal to (Number of Exceptions supported x 4), with a minimum alignment
// of 128 bytes.". The array is aligned in linker.ld, using the dedicated section ".vector_table".

// The exception vector table contains a stack pointer, 15 exception handlers, and an entry for
// each interrupt.
#define INTERRUPT_COUNT 100
#define EXCEPTION_COUNT (16 + INTERRUPT_COUNT)
#define INT_TO_EXC(i_) (16 + (i_))
uintptr_t __isr_vector[EXCEPTION_COUNT] __attribute__((section(".vector_table"))) __attribute__((used)) = {
	[0] = (uintptr_t)&StackTop,				/* Top of Stack */
	[1] = (uintptr_t)RTCoreMain,			/* Reset Handler */
	[2] = (uintptr_t)NMI_Handler,			/* NMI Handler */
	[3] = (uintptr_t)Hard_Fault_Handler,	/* Hard Fault Handler */
	[4] = (uintptr_t)MPU_Fault_Handler,		/* MPU Fault Handler */
	[5] = (uintptr_t)Bus_Fault_Handler,		/* Bus Fault Handler */
	[6] = (uintptr_t)Usage_Fault_Handler,	/* Usage Fault Handler */
	[11] = (uintptr_t)SVCall__Handler,		/* SVCall Handler */
	[12] = (uintptr_t)Debug_Monitor_Handler,/* Debug Monitor Handler */
	[14] = (uintptr_t)Pend_SV__Handler,		/* PendSV Handler */
	[15] = (uintptr_t)Sys_Tick_Handler,		/* SysTick Handler */

	[INT_TO_EXC(0)... INT_TO_EXC(INTERRUPT_COUNT - 1)] = (uintptr_t)DefaultExceptionHandler
};

