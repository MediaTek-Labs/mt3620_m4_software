# Sample: MT3620 M4 real-time application - Bare Metal Hello World

### Description

This sample app for an MT3620 real-time core repeatedly transmits a simple message over a UART. These messages can be read in terminal application on a PC using a USB-to-serial adapter. By default, it uses the ISU0 UART interface, but if your hardware doesn't expose this UART's TX pin, then the sample can be altered to use a different UART.

Output example:
![image](https://github.com/LawranceLiu/Azure-Sphere-MT3620-M4-Samples/blob/master/MT3620_M4_Sample_Code/BareMetal/MT3620_RTApp_BareMetal_HelloWorld/pic/hello_world.jpg)

Please refer to the [MT3620 M4 API Rerference Manual](https://support.mediatek.com/AzureSphere/mt3620/M4_API_Reference_Manual) for the detailed API description.
    

## Prerequisites

* **Hardware**
    * [AVNET MT3620 Starter Kit](https://www.avnet.com/shop/us/products/avnet-engineering-services/aes-ms-mt3620-sk-g-3074457345636825680/)
        * Connect PC UART Rx to AVNET MT3620 Starter Kit Tx of Click #1
        * ![image](https://github.com/LawranceLiu/Azure-Sphere-MT3620-M4-Samples/blob/master/MT3620_M4_Sample_Code/BareMetal/MT3620_RTApp_BareMetal_HelloWorld/pic/avnet.jpg)
        * ![image](https://github.com/LawranceLiu/Azure-Sphere-MT3620-M4-Samples/blob/master/MT3620_M4_Sample_Code/BareMetal/MT3620_RTApp_BareMetal_HelloWorld/pic/avnet_connect.jpg)
    * or [Seeed MT3620 Development Kit](https://aka.ms/azurespheredevkits)
        * Connect PC UART Rx to Seeed MT3620 Development Kit GPIO 26 / TXD0
        * ![image](https://github.com/LawranceLiu/Azure-Sphere-MT3620-M4-Samples/blob/master/MT3620_M4_Sample_Code/BareMetal/MT3620_RTApp_BareMetal_HelloWorld/pic/seeed.jpg)
        * ![image](https://github.com/LawranceLiu/Azure-Sphere-MT3620-M4-Samples/blob/master/MT3620_M4_Sample_Code/BareMetal/MT3620_RTApp_BareMetal_HelloWorld/pic/seeed_connect.jpg)
    * or other hardware that implements the [MT3620 Reference Development Board (RDB)](https://docs.microsoft.com/azure-sphere/hardware/mt3620-reference-board-design) design.

* **Software**
    * Refer to [Azure Sphere software installation guide](https://docs.microsoft.com/en-ca/azure-sphere/install/overview).
    * A terminal emulator (such as Telnet or [PuTTY](https://www.chiark.greenend.org.uk/~sgtatham/putty/) to display the output).
