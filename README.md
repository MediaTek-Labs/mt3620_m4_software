# MediaTek MT3620 M4 Driver & Real-Time Application Sample Code (Beta)
### Current Status
* FreeRTOS BSP is supported.
* Available **Bare Metal** sample codes: GPIO / Hello World
* Available **FreeRTOS** sample codes: GPIO / GPT / UART

### To clone this repository:
```
git clone https://github.com/LawranceLiu/Azure-Sphere-MT3620-M4-Samples.git
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
    * Bot **Bare Metal** and **FreeRTOS** sample code are included.  

Please refer to the **[MT3620 M4 API Reference Manual](https://support.mediatek.com/AzureSphere/mt3620/M4_API_Reference_Manual)** for the detailed API description.  
Use of the real-time cores is currently a **Beta** feature.

### Prerequisites
* **Hardware**
    * [AVNET MT3620 Starter Kit](https://www.avnet.com/shop/us/products/avnet-engineering-services/aes-ms-mt3620-sk-g-3074457345636825680/)
    * or [Seeed MT3620 Development Kit](https://aka.ms/azurespheredevkits)
    * or other hardware that implements the [MT3620 Reference Development Board (RDB)](https://docs.microsoft.com/azure-sphere/hardware/mt3620-reference-board-design) design.
* **Software**
    * Refer to [Azure Sphere software installation guide](https://docs.microsoft.com/en-ca/azure-sphere/install/overview).
    * A terminal emulator (such as Telnet or [PuTTY](https://www.chiark.greenend.org.uk/~sgtatham/putty/) to display the output log).