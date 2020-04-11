#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>

#include "FreeRTOS.h"
#include "task.h"
#include "printf.h"
extern "C" {
#include "mt3620.h"
}

extern "C" {
#include "os_hal_uart.h"
#include "os_hal_gpio.h"
}

#include "DigitalOut.h"

extern "C" {
    void vApplicationStackOverflowHook(TaskHandle_t xTask, char* pcTaskName);
    void vApplicationMallocFailedHook(void);
    _Noreturn void RTCoreMain(void);
}

static const UART_PORT UART_PUTCHAR = OS_HAL_UART_PORT0;
static const os_hal_gpio_pin GPIO_LED = OS_HAL_GPIO_16;

void _putchar(char character)
{
	mtk_os_hal_uart_put_char(UART_PUTCHAR, character);
	if (character == '\n')
		mtk_os_hal_uart_put_char(UART_PUTCHAR, '\r');
}

void vApplicationStackOverflowHook(TaskHandle_t xTask, char* pcTaskName)
{
	printf("%s: %s\n", __func__, pcTaskName);
}

void vApplicationMallocFailedHook(void)
{
	printf("%s\n", __func__);
}

static void BlinkTask(void* pParameters)
{
	printf("Blink Task Started\n");

    DigitalOut led(GPIO_LED);

	while (true)
	{
        led.Write(0);
		vTaskDelay(pdMS_TO_TICKS(200));
        led.Write(1);
		vTaskDelay(pdMS_TO_TICKS(800));
	}
}

_Noreturn void RTCoreMain(void)
{
	NVIC_SetupVectorTable();

	mtk_os_hal_uart_ctlr_init(UART_PUTCHAR);

	xTaskCreate(BlinkTask, "Blink Task", 1024 / 4, NULL, 4, NULL);
	vTaskStartScheduler();

	for (;;)
    {
		__asm__("wfi");
	}
}
