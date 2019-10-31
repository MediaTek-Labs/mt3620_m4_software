#  Copyright (c) Microsoft Corporation. All rights reserved.
#  Licensed under the MIT License.

CMAKE_MINIMUM_REQUIRED(VERSION 3.11)
PROJECT(MT3620_RTApp_BareMetal_HelloWorld C)

# Create executable
ADD_COMPILE_DEFINITIONS(OS_BARE_METAL)
ADD_EXECUTABLE(${PROJECT_NAME} main.c os_hal_uart.c os_hal_gpio.c)
ADD_SUBDIRECTORY(../../../MT3620_M4_Driver ../../../MT3620_M4_Driver/out)
INCLUDE_DIRECTORIES(../../../MT3620_M4_Driver/MHAL/inc)
TARGET_LINK_LIBRARIES(${PROJECT_NAME} MT3620_M4_Driver_UART MT3620_M4_Driver_GPIO MT3620_M4_Driver_OSAI)
SET_TARGET_PROPERTIES(${PROJECT_NAME} PROPERTIES LINK_DEPENDS ${CMAKE_SOURCE_DIR}/linker.ld)

# Add MakeImage post-build command
INCLUDE("${AZURE_SPHERE_MAKE_IMAGE_FILE}")
