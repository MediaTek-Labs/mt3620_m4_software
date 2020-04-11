#pragma once

extern "C" {
#include "os_hal_gpio.h"
}

class DigitalOut
{
private:
	os_hal_gpio_pin _Pin;

public:
	DigitalOut(os_hal_gpio_pin pin);
	void Write(int value);

};
