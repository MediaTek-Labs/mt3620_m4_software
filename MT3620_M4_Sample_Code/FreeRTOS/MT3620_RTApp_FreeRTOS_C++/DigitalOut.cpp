#include "DigitalOut.h"
#include <cstdlib>

extern "C" {
#include "os_hal_gpio.h"
}

DigitalOut::DigitalOut(os_hal_gpio_pin pin)
{
	_Pin = pin;
	if (mtk_os_hal_gpio_request(_Pin) != 0) abort();
	mtk_os_hal_gpio_set_direction(_Pin, OS_HAL_GPIO_DIR_OUTPUT);
}

void DigitalOut::Write(int value)
{
	mtk_os_hal_gpio_set_output(_Pin, value ? OS_HAL_GPIO_DATA_HIGH : OS_HAL_GPIO_DATA_LOW);
}
