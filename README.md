# MediaTek MT3620 M4 Driver & Real-Time Application Sample Code
### Current Status
* Avaiable sample code
    * **FreeRTOS**: GPIO / GPT / UART / SPIM / PWM / I2C / I2S / DMA / ADC / MBOX / LP(Low Power) / C++ / Arducam / Arducam+TFT_Display / Accelerometer
    * **Bare Metal**: GPIO / Hello World / MBOX
* Supported Azure Sphere SDK/API Version
    * SDK Version: **20.04** (Download latest version [here](https://docs.microsoft.com/en-ca/azure-sphere/install/install-sdk#install-the-azure-sphere-sdk).)
    * API Version: **5+Beta2004**
* Revision History of relesae_200520
    * Azure Sphere SDK 20.04 (API version "5+Beta2004") is supported.
    * FreeRTOS and Bare Metal OS_HAL are merged.
    * FreeRTOS RTApp Toolchain cmake file is updated.
    * New sample code: Bare Metal Mailbox
    * New sample code: FreeRTOS Arducam+TFT_Display
* Known Issue
    * External interrupt is not working.
        * Caused by Azure Sphere OS firewall configuration, still under discussion/clarification.
    * SPI_Slave is not working.
        * App Manifest file does not support SPI slave, still under discussion/clarification.
    * WDT_Reset (WatchDog Timer Reset) failed to reboot correctly.
        * M4 core failed to reboot after WDT_Reset, still under discussion/clarification.

### To clone this repository:
```
git clone https://github.com/MediaTek-Labs/mt3620_m4_software.git
```

### Description
This repository maintains the MT3620 M4 driver and real-time application sample code, which divided into the following directories:
* **MT3620_M4_BSP/**
    * This folder includes the CMSIS-Core APIs and the configuration of interrupt vector table.
    * Current BSP supports **Bare Metal** and **FreeRTOS**.  
* **MT3620_M4_Driver/**
    * The MT3620 M4 driver provides the APIs to access the peripheral interfaces, ex GPIO / SPI / I2S / I2C / UART...
    * This driver could be divided into two layers
        * Upper layer: **M-HAL** (MediaTek Hardware AbstractionLayer), which provides high-level API to real-time application.
        * Lower layer: **HDL** (Hardware Driving Layer), which handles the low-level hardware control.  
* **MT3620_M4_Sample_Code/**
    * This is the executable CMake project sample code which utilizes the M-HAL APIs to access the peripheral interfaces.
    * Both **Bare Metal** and **FreeRTOS** sample code are included.  

Please refer to the **[MT3620 M4 API Reference Manual](https://support.mediatek.com/AzureSphere/mt3620/M4_API_Reference_Manual)** for the detailed API description.  

### Prerequisites
* **Hardware**
    * [AVNET MT3620 Starter Kit](https://www.avnet.com/shop/us/products/avnet-engineering-services/aes-ms-mt3620-sk-g-3074457345636825680/) or [Seeed MT3620 Development Kit](https://www.seeedstudio.com/Azure-Sphere-MT3620-Development-Kit-US-Version-p-3052.html)
* **Software**
    * Refer to [Azure Sphere software installation guide](https://docs.microsoft.com/en-ca/azure-sphere/install/overview).
    * A terminal emulator (such as Telnet or [PuTTY](https://www.chiark.greenend.org.uk/~sgtatham/putty/) to display the output log).