# This code is based on a sample from Microsoft (see license below),
# with modifications made by MediaTek.
# Modified version of CMakeLists.txt from Microsoft Azure Sphere sample code:
# https://github.com/Azure/azure-sphere-samples/blob/master/Samples/HelloWorld/HelloWorld_RTApp_MT3620_BareMetal/CMakeLists.txt

#  Copyright (c) Microsoft Corporation. All rights reserved.
#  Licensed under the MIT License.

cmake_minimum_required(VERSION 3.10)

# Configurations
project(FreeRTOS_RTcore_Arducam_ILI9341 C)
azsphere_configure_tools(TOOLS_REVISION "20.07")
add_compile_definitions(OSAI_FREERTOS)
add_compile_definitions(OSAI_ENABLE_DMA)
add_compile_definitions(AzureSphere_CM4)
# add_compile_definitions(IMG_FMT_BMP)
# When place CODE_REGION in FLASH instead of TCM, please enable this definition:
# add_compile_definitions(M4_ENABLE_XIP_FLASH)
add_link_options(-specs=nano.specs -specs=nosys.specs)

# Executable
add_executable(${PROJECT_NAME}
               main.c
               ./arducam_driver/arducam_driver.c
               ./ili9341_driver/font.c
               ./ili9341_driver/ili9341.c
               ./ili9341_driver/ili9341_ll.c
               ./ili9341_driver/text.c
               ./jpeg_decoder/tjpgd.c
               ../../OS_HAL/src/os_hal_gpio.c
               ../../OS_HAL/src/os_hal_uart.c
               ../../OS_HAL/src/os_hal_dma.c
               ../../OS_HAL/src/os_hal_i2c.c
               ../../OS_HAL/src/os_hal_spim.c)

# Include Folders
include_directories(${PROJECT_NAME} PUBLIC
                    ./)
target_include_directories(${PROJECT_NAME} PUBLIC
                           ../../OS_HAL/inc
                           ./arducam_driver
                           ./ili9341_driver
                           ./jpeg_decoder
                           ./)

# Libraries
set(OSAI_FREERTOS 1)
add_subdirectory(../../../MT3620_M4_Driver ./lib/MT3620_M4_Driver)
target_link_libraries(${PROJECT_NAME} MT3620_M4_Driver)

# Linker, Image
set_target_properties(${PROJECT_NAME} PROPERTIES LINK_DEPENDS ${CMAKE_SOURCE_DIR}/linker.ld)
azsphere_target_add_image_package(${PROJECT_NAME})
