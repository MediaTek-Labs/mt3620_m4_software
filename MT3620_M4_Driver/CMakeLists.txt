CMAKE_MINIMUM_REQUIRED(VERSION 3.11)

INCLUDE_DIRECTORIES(./MHAL/inc ./HDL/inc)

ADD_LIBRARY(MT3620_M4_Driver_OSAI ./MHAL/src/mhal_osai.c)
ADD_LIBRARY(MT3620_M4_Driver_UART ./MHAL/src/mhal_uart.c ./HDL/src/hdl_uart.c)
ADD_LIBRARY(MT3620_M4_Driver_GPIO ./MHAL/src/mhal_gpio.c ./HDL/src/hdl_gpio.c)

