# Sample: MT3620 M4 real-time application - FreeRTOS I2C
### Description
This sample demonstrates how to use I2C on an MT3620 real-time core.  
- ISU0 UART interface is used to print the output log.
- ISU1 I2C interface is used as I2C MASTER.
- ISU2 I2C interface is used as I2C SLAVE.
- This sample code is trying to demostrate I2C loopback test on ISU1/ISU2. Please connect ISU1(SDA/SCL) to ISU2(SDA/SCL).  
(Note, This sample code works only on Seeed development board, NOT on AVNET development board. Since ISU2 of AVNET development board has additional sensors connected, if run this sample code on AVNET development board, some I2C transport error might be happening. One possible work around for AVNET development board is to change I2C_MAX_LEN from 64 to 8.)  
(Note, UART port number in main.c could be changed from **OS_HAL_UART_ISU0** to **OS_HAL_UART_PORT0** to use M4 dedicate UART port.)  
Please refer to the [MT3620 M4 API Rerference Manual](https://support.mediatek.com/AzureSphere/mt3620/M4_API_Reference_Manual) for the detailed API description.

### Prerequisites
* **Hardware**
    * [AVNET MT3620 Starter Kit](https://www.avnet.com/shop/us/products/avnet-engineering-services/aes-ms-mt3620-sk-g-3074457345636825680/) or [Seeed MT3620 Development Kit](https://www.seeedstudio.com/Azure-Sphere-MT3620-Development-Kit-US-Version-p-3052.html)
* **Software**
    * Refer to [Azure Sphere software installation guide](https://docs.microsoft.com/en-ca/azure-sphere/install/overview).
    * A terminal emulator (such as Telnet or [PuTTY](https://www.chiark.greenend.org.uk/~sgtatham/putty/) to display the output log).

### How to build and run the sample
1. Start Visual Studio.  
2. From **File** menu, select **Open > CMake...** and navigate to the folder that contains this sample.  
3. Select **CMakeList.txt** and then click **Open**.  
4. Wait few seconds until Visual Studio finish create the project files.
5. From **Build** menu, select **Build ALL (Ctrl+Shift+B)**.  
6. Click **Select Start Item** and then select **GDB Debugger (RTCore)** as following.  
    ![VS Start](../../BareMetal/MT3620_RTApp_BareMetal_HelloWorld/pic/select_start_item.jpg)
7. Press **F5** to start the application with debugging.  

### Hardware configuration
* [AVNET MT3620 Starter Kit](https://www.avnet.com/shop/us/products/avnet-engineering-services/aes-ms-mt3620-sk-g-3074457345636825680/)
    * Connect ISU1_I2C to ISU2_I2C:
        ![AVNET I2C Loopback](../../BareMetal/MT3620_RTApp_BareMetal_HelloWorld/pic/avnet_i2c_loopback.png)
    * Connect PC UART Rx to AVNET MT3620 Starter Kit Click #1 TX (ISU0_UART_TX):
        ![AVNET UART](../../BareMetal/MT3620_RTApp_BareMetal_HelloWorld/pic/avnet_uart.png)
* [Seeed MT3620 Development Kit](https://www.seeedstudio.com/Azure-Sphere-MT3620-Development-Kit-US-Version-p-3052.html)
    * Connect ISU1_I2C to ISU2_I2C:
        ![Seeed I2C Loopback](../../BareMetal/MT3620_RTApp_BareMetal_HelloWorld/pic/seeed_i2c_loopback.png)
    * Connect PC UART Rx to Seeed MT3620 Development Kit GPIO 26 / TXD0  (ISU0_UART_TX)
        ![Seeed UART](../../BareMetal/MT3620_RTApp_BareMetal_HelloWorld/pic/seeed_uart.png)
