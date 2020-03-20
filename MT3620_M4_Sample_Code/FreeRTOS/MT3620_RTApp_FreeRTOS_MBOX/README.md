# Sample: MT3620 M4 real-time application - FreeRTOS MBOX
### Description
This sample demonstrates how to use MBOX on an MT3620 real-time core.
- There are 1 HLApp and 2 RTApp (RTAppA & RTAppB) included in this sample code.
- Both RTAppA and RTAppB listens for incoming mailbox message from HLApp, once RTAppA or RTAppB receives mailbox message from HLApp, message content is printed to UART and then send back to HLApp.
- ISU0 UART interface is used by RTAppA to print the output log.
- ISU1 UART interface is used by RTAppB to print the output log.  
(Note, UART port number in main.c could be changed from **OS_HAL_UART_ISU0** to **OS_HAL_UART_PORT0** to use M4 dedicate UART port.)  
Please refer to the [MT3620 M4 API Rerference Manual](https://support.mediatek.com/AzureSphere/mt3620/M4_API_Reference_Manual) for the detailed API description.

### Prerequisites
* **Hardware**
    * [AVNET MT3620 Starter Kit](https://www.avnet.com/shop/us/products/avnet-engineering-services/aes-ms-mt3620-sk-g-3074457345636825680/)
    * or [Seeed MT3620 Development Kit](https://aka.ms/azurespheredevkits)
    * or other hardware that implements the [MT3620 Reference Development Board (RDB)](https://docs.microsoft.com/azure-sphere/hardware/mt3620-reference-board-design) design.
* **Software**
    * Refer to [Azure Sphere software installation guide](https://docs.microsoft.com/en-ca/azure-sphere/install/overview).
    * A terminal emulator (such as Telnet or [PuTTY](https://www.chiark.greenend.org.uk/~sgtatham/putty/) to display the output log).

### How to build and run the sample
* **Build HLApp and RTApp**  
    1. Important Note! To reuse the official GCC Cortex-M4F port, the gcc compiler flag should be modified to use FPU instructions. **Please copy the *AzureSphereRTCoreToolchainVFP.cmake* file into the Azure Sphere SDK install folder.** (Default path is *C:\Program Files (x86)\Microsoft Azure Sphere SDK\CMakeFiles*)
    2. Start Visual Studio. (Totally three Visual Studio instances for HLApp/RTAppA/RTAppB.)
    3. From **File** menu, select **Open > CMake...** and navigate to the folder that contains this sample.
    4. Select **CMakeList.txt** and then click **Open**.
    5. Wait few seconds until Visual Studio finish create the project files.
    6. From **Build** menu, select **Build ALL (Ctrl+Shift+B)**.

* **Download RTApp**  
    Enter the following commands in the "Azure Sphere Developer Command Prompt":
    1. azsphere dev sideload delete
    2. azsphere dev sideload deploy --imagepackage .\MT3620_RTApp_FreeRTOS_MBOX\RTAppA\out\ARM-Debug-4+Beta2001\Azure_Sphere_RTcore_FreeRTOS_MBOX_A.imagepackage
    3. azsphere dev sideload deploy --imagepackage .\MT3620_RTApp_FreeRTOS_MBOX\RTAppB\out\ARM-Debug-4+Beta2001\Azure_Sphere_RTcore_FreeRTOS_MBOX_B.imagepackage
    4. azsphere dev app show-status

* **Run the sample**  
    1. From the Visual Studio instance of the HLApp, click **Select Start Item** and then select **GDB Debugger (HLCore)**.  
    2. Press **F5** to start the application with debugging.  
    3. Check Visual Studio log and the UART log of ISU0 & ISU1.

### Hardware configuration
* [AVNET MT3620 Starter Kit](https://www.avnet.com/shop/us/products/avnet-engineering-services/aes-ms-mt3620-sk-g-3074457345636825680/)
    * Connect PC UART Rx to AVNET MT3620 Starter Kit Click #1 TX (ISU0_UART_TX):
        ![image](https://raw.githubusercontent.com/MediaTek-Labs/mt3620_m4_software/master/MT3620_M4_Sample_Code/BareMetal/MT3620_RTApp_BareMetal_HelloWorld/pic/avnet_uart.png)
* [Seeed MT3620 Development Kit](https://aka.ms/azurespheredevkits)
    * Connect PC UART Rx to Seeed MT3620 Development Kit GPIO 26 / TXD0  (ISU0_UART_TX)
        ![image](https://raw.githubusercontent.com/MediaTek-Labs/mt3620_m4_software/master/MT3620_M4_Sample_Code/BareMetal/MT3620_RTApp_BareMetal_HelloWorld/pic/seeed_uart.png)
