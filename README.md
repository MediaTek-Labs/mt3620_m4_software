# MediaTek MT3620 M4 Driver & Real-Time Application Sample Code (Beta)

### To clone this repository:
```
git clone https://github.com/LawranceLiu/Azure-Sphere-MT3620-M4-Samples.git
```

### Description
This repository maintains the MT3620 M4 driver and real-time application sample code, which divided into the following directories:

* **MT3620\_M4_Driver/**
    * The MT3620 M4 Driver which drives the external peripherals via GPIO or SPI or I2S or I2C or UART...
    * This driver could be divided into two layers
        * Upper layer: **M-HAL** (MediaTek Hardware AbstractionLayer), which provides high-level API to real-time application.
        * Lower layer: **HDL** (Hardware Driver Layer), which handles the low-level hardware control.

* **Sample_Code/**
    * The sample code which uses the **M-HAL** API in MT3620_M4_Driver/ to control the external peripherals.

Please refer to the **[MT3620 M4 API Reference Manual](https://support.mediatek.com/AzureSphere/mt3620/)** for the detailed API description.
Use of the real-time cores is currently a **Beta** feature.

### Prerequisites
* **Hardware**
    * [AVNET MT3620 Starter Kit](https://www.avnet.com/shop/us/products/avnet-engineering-services/aes-ms-mt3620-sk-g-3074457345636825680/)
    * or [Seeed MT3620 Development Kit](https://aka.ms/azurespheredevkits)
    * or other hardware that implements the [MT3620 Reference Development Board (RDB)](https://docs.microsoft.com/azure-sphere/hardware/mt3620-reference-board-design) design.

* **Software**
    * Refer to [Azure Sphere software installation guide](https://docs.microsoft.com/en-ca/azure-sphere/install/overview).
