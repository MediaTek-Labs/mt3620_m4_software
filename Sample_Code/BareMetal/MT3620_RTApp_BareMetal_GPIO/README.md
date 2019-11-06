# Sample: MT3620 M4 real-time application - Bare Metal GPIO

### Description

This sample demonstrates how to use GPIO on an MT3620 real-time core.

- It provides access to one of the LEDs on the MT3620 development board using GPIO.
- It uses a button to change the blink rate of the LED.

It runs directly on one of the MT3620 real-time cores(M4) instead of the high-level core(A7).
Please refer to the [MT3620 M4 API Rerference Manual](https://support.mediatek.com/AzureSphere/mt3620/M4_API_Reference_Manual) for the detailed API description.

### Prerequisites
* **Hardware**
    * [AVNET MT3620 Starter Kit](https://www.avnet.com/shop/us/products/avnet-engineering-services/aes-ms-mt3620-sk-g-3074457345636825680/)
    * or [Seeed MT3620 Development Kit](https://aka.ms/azurespheredevkits)
    * or other hardware that implements the [MT3620 Reference Development Board (RDB)](https://docs.microsoft.com/azure-sphere/hardware/mt3620-reference-board-design) design.

* **Software**
    * Refer to [Azure Sphere software installation guide](https://docs.microsoft.com/en-ca/azure-sphere/install/overview).
