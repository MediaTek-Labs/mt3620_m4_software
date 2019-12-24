# Sample: MT3620 M4 real-time application - FreeRTos GPIO

### Description

This sample demonstrates how to use GPIO on an MT3620 real-time core.

- GPIO_8 and GPIO_9 for on-board LED control (Red and Green).
- GPIO_12 and GPIO_13 for on-board Button control (Button A and Button B).


It runs directly on one of the MT3620 real-time cores(M4) instead of the high-level core(A7).
Please refer to the [MT3620 M4 API Rerference Manual](https://support.mediatek.com/AzureSphere/mt3620/M4_API_Reference_Manual) for the detailed API description.

### Prerequisites
* **Hardware**
    * [AVNET MT3620 Starter Kit](https://www.avnet.com/shop/us/products/avnet-engineering-services/aes-ms-mt3620-sk-g-3074457345636825680/)
    * or [Seeed MT3620 Development Kit](https://aka.ms/azurespheredevkits)
    * or other hardware that implements the [MT3620 Reference Development Board (RDB)](https://docs.microsoft.com/azure-sphere/hardware/mt3620-reference-board-design) design.

* **Software**
    * Refer to [Azure Sphere software installation guide](https://docs.microsoft.com/en-ca/azure-sphere/install/overview).

### How to build and run the sample
0. Important Note! To reuse the official GCC Cortex-M4F port, the gcc compiler flag should be modified to use FPU instructions. **Please copy the *AzureSphereRTCoreToolchainVFP.cmake* file into the Azure Sphere SDK install folder.** (Default path is *C:\Program Files (x86)\Microsoft Azure Sphere SDK\CMakeFiles*)
1. Start Visual Studio.  
2. From **File** menu, select **Open > CMake...** and navigate to the folder that contains this sample.  
3. Select **CMakeList.txt** and then click **Open**.  
4. Wait few seconds until Visual Studio finish create the project files.
5. From **Build** menu, select **Build ALL (Ctrl+Shift+B)**.  
6. Click **Select Start Item** and then select **GDB Debugger (RTCore)** as following.  
    ![image](https://github.com/LawranceLiu/Azure-Sphere-MT3620-M4-Samples/blob/master/MT3620_M4_Sample_Code/BareMetal/MT3620_RTApp_BareMetal_HelloWorld/pic/select_start_item.jpg)  
7. Press **F5** to start the application with debugging.  
