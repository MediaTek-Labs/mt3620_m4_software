# Sample: MT3620 M4 real-time application - FreeRTOS Arducam
### Description
This sample demonstrates how to control Arducam/ILI9341 TFT LCD Display on an MT3620 real-time core.  
- M4 dedicate UART interface is used to print the output log.  
- ISU0 I2C interface is used to control Arducam.
- ISU1 SPI interface is used to receive image data from Arducam.
- ISU3 SPI interface is used to control the ILI9341 TFT LCD Display.
- GPIO_0 / GPIO_1 / GPIO_2 are used to control ILI9341 TFT LCD Display.
- On-board button_A is used to trigger Arducam "single capture".
- On-board button_B is used to trigger Arducam "continuous capture".
- ![VS Start](../../BareMetal/MT3620_RTApp_BareMetal_HelloWorld/pic/seeed_arducam_ili9341.png)
Please refer to the [MT3620 M4 API Reference Manual](https://support.mediatek.com/AzureSphere/mt3620/M4_API_Reference_Manual) for the detailed API description.

### Prerequisites
* **Hardware**
    * [Arducam 2MP OV2640 Mini Camera Module](https://www.arducam.com/product/arducam-2mp-spi-camera-b0067-arduino/)
    * [ILI9341 TFT LCD Display](https://www.amazon.com/HiLetgo-240X320-Resolution-Display-ILI9341/dp/B073R7BH1B)
    * [Seeed MT3620 Development Kit](https://www.seeedstudio.com/Azure-Sphere-MT3620-Development-Kit-US-Version-p-3052.html)

* **Software**
    * Refer to [Azure Sphere software installation guide](https://docs.microsoft.com/en-ca/azure-sphere/install/overview).
    * A terminal emulator (such as Telnet or [PuTTY](https://www.chiark.greenend.org.uk/~sgtatham/putty/) to display the output log).

### How to build and run the sample
1. Start Visual Studio.  
2. From **File** menu, select **Open > CMake...** and navigate to the folder that contains this sample.  
3. Select **CMakeList.txt** and then click **Open**.  
4. Wait few seconds until Visual Studio finishes creating the project files.
5. From **Build** menu, select **Build ALL (Ctrl+Shift+B)**.  
6. Click **Select Start Item** and then select **GDB Debugger (RTCore)** as following.  
    ![VS Start](../../BareMetal/MT3620_RTApp_BareMetal_HelloWorld/pic/select_start_item.jpg)
7. Press **F5** to start the application with debugging.  

### Hardware configuration
* [Seeed MT3620 Development Kit](https://www.seeedstudio.com/Azure-Sphere-MT3620-Development-Kit-US-Version-p-3052.html)
    * Connect Arducam:
        ![Seeed Arducam no Host](../../BareMetal/MT3620_RTApp_BareMetal_HelloWorld/pic/seeed_arducam_no_host.png)
    * Connect ILI9341:
        ![Seeed ILI9341](../../BareMetal/MT3620_RTApp_BareMetal_HelloWorld/pic/seeed_ili9341.png)
    * All Pins Definition:
        ![Seeed Pins](../../BareMetal/MT3620_RTApp_BareMetal_HelloWorld/pic/seeed_pins.png)
