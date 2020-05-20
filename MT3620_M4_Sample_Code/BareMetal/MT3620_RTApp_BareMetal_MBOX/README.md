# Sample: MT3620 M4 real-time application - Bare Metal MBOX
### Description
This sample demonstrates how to use MBOX on an MT3620 real-time core.
- There are 1 HLApp and 2 RTApp (RTAppA & RTAppB) included in this sample code.
- Both RTAppA and RTAppB listens for incoming mailbox message from HLApp, once RTAppA or RTAppB receives mailbox message from HLApp, message content is printed to UART and then send back to HLApp.
- The RTAppA also sends mailbox message to RTAppB, once RTAppB receives mailbox message from RTAppA, message content is printed to UART and then send back to RTAppA.
- Once RTAppA receives mailbox message from RTAppB, message content is printed to UART.
- ISU0 UART interface is used by RTAppA to print the output log.
- ISU1 UART interface is used by RTAppB to print the output log.  
(Note, UART port number in main.c could be changed from **OS_HAL_UART_ISU0** to **OS_HAL_UART_PORT0** to use M4 dedicate UART port.)  
Please refer to the [MT3620 M4 API Rerference Manual](https://support.mediatek.com/AzureSphere/mt3620/M4_API_Reference_Manual) for the detailed API description.

### Prerequisites
* **Hardware**
    * [AVNET MT3620 Starter Kit](https://www.avnet.com/shop/us/products/avnet-engineering-services/aes-ms-mt3620-sk-g-3074457345636825680/) or [Seeed MT3620 Development Kit](https://www.seeedstudio.com/Azure-Sphere-MT3620-Development-Kit-US-Version-p-3052.html)
* **Software**
    * Refer to [Azure Sphere software installation guide](https://docs.microsoft.com/en-ca/azure-sphere/install/overview).
    * A terminal emulator (such as Telnet or [PuTTY](https://www.chiark.greenend.org.uk/~sgtatham/putty/) to display the output log).

### How to build and run the sample
* **Build HLApp and RTApp**  
    1. Start Visual Studio. (Totally three Visual Studio instances for HLApp/RTAppA/RTAppB.)
    2. From **File** menu, select **Open > CMake...** and navigate to the folder that contains this sample.
    3. Select **CMakeList.txt** and then click **Open**.
    4. Wait few seconds until Visual Studio finish create the project files.
    5. From **Build** menu, select **Build ALL (Ctrl+Shift+B)**.
    6. Repeat the above for RTAppA and RTAppB and HLApp.

* **Download RTApp**  
    Enter the following commands in the "Azure Sphere Developer Command Prompt":
```
azsphere dev sideload delete
azsphere dev sideload deploy --imagepackage .\RTAppA\out\ARM-Debug\Bare_Metal_RTApp_Mbox_A.imagepackage
azsphere dev sideload deploy --imagepackage .\RTAppB\out\ARM-Debug\Bare_Metal_RTApp_Mbox_B.imagepackage
azsphere dev app show-status
```

* **Run the sample**  
    1. From the Visual Studio instance of the HLApp, click **Select Start Item** and then select **GDB Debugger (HLCore)**.  
    2. Press **F5** to start the application with debugging.  
    3. Check Visual Studio log and the UART log of ISU0 & ISU1.

### Hardware configuration
* [AVNET MT3620 Starter Kit](https://www.avnet.com/shop/us/products/avnet-engineering-services/aes-ms-mt3620-sk-g-3074457345636825680/)
    * Connect PC UART Rx to AVNET MT3620 Starter Kit Click #1 TX (ISU0_UART_TX):
        ![AVNET UART](../../BareMetal/MT3620_RTApp_BareMetal_HelloWorld/pic/avnet_uart.png)
* [Seeed MT3620 Development Kit](https://www.seeedstudio.com/Azure-Sphere-MT3620-Development-Kit-US-Version-p-3052.html)
    * Connect PC UART Rx to Seeed MT3620 Development Kit GPIO 26 / TXD0  (ISU0_UART_TX)
        ![Seeed UART](../../BareMetal/MT3620_RTApp_BareMetal_HelloWorld/pic/seeed_uart.png)
