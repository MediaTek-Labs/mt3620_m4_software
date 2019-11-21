CMAKE_MINIMUM_REQUIRED(VERSION 3.11)

INCLUDE_DIRECTORIES(./CMSIS/include ./mt3620/inc)

ADD_LIBRARY(MT3620_M4_BSP ./mt3620/src/nvic.c ./mt3620/src/cirq_common.c ./mt3620/src/vector_table.c)


