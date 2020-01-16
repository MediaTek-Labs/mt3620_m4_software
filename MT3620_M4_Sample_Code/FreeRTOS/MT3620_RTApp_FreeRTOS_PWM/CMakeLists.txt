# This code is based on a sample from Microsoft (see license below),
# with modifications made by MediaTek.
# Modified version of CMakeLists.txt from Microsoft Azure Sphere sample code:
# https://github.com/Azure/azure-sphere-samples/blob/master/Samples/HelloWorld/HelloWorld_RTApp_MT3620_BareMetal/CMakeLists.txt

#  Copyright (c) Microsoft Corporation. All rights reserved.
#  Licensed under the MIT License.

CMAKE_MINIMUM_REQUIRED(VERSION 3.11)

# Configurations
PROJECT(Azure_Sphere_RTcore_FreeRTOS_PWM C)
ADD_COMPILE_DEFINITIONS(OSAI_FREERTOS)
ADD_LINK_OPTIONS(-specs=nano.specs -specs=nosys.specs)

# Include Folders
INCLUDE_DIRECTORIES(
		../OS_HAL/inc
		../../../MT3620_M4_BSP/printf
		../../../MT3620_M4_BSP/freertos/portable
		../../../MT3620_M4_BSP/freertos/include
		../../../MT3620_M4_Driver/MHAL/inc
		../../../MT3620_M4_BSP/mt3620/inc
		../../../MT3620_M4_BSP/CMSIS/include
		./)

# Executable
ADD_EXECUTABLE(${PROJECT_NAME} main.c
                               ../../../MT3620_M4_BSP/printf/printf.c
                               ../OS_HAL/src/os_hal_gpio.c
                               ../OS_HAL/src/os_hal_uart.c
                               ../OS_HAL/src/os_hal_pwm.c)

# Libraries
ADD_SUBDIRECTORY(../../../MT3620_M4_Driver ./lib/MT3620_M4_Driver)
ADD_SUBDIRECTORY(../../../MT3620_M4_BSP ./lib/MT3620_M4_BSP)
ADD_SUBDIRECTORY(../../../MT3620_M4_BSP/FreeRTOS ./lib/FreeRTOS)
TARGET_LINK_LIBRARIES(${PROJECT_NAME} MT3620_M4_Driver_GPIO
                                      MT3620_M4_Driver_UART
                                      MT3620_M4_Driver_OSAI
                                      MT3620_M4_Driver_PWM
                                      MT3620_M4_BSP
                                      FreeRTOS)

# Linker, Image
SET_TARGET_PROPERTIES(${PROJECT_NAME} PROPERTIES LINK_DEPENDS ${CMAKE_SOURCE_DIR}/linker.ld)
INCLUDE("${AZURE_SPHERE_MAKE_IMAGE_FILE}")