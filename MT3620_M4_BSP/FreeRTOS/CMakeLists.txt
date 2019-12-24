CMAKE_MINIMUM_REQUIRED(VERSION 3.11)

INCLUDE_DIRECTORIES(./include ./portable)

ADD_LIBRARY(FreeRTOS ./croutine.c ./event_groups.c ./list.c ./queue.c ./stream_buffer.c ./tasks.c ./timers.c ./portable/heap_4.c ./portable/port.c)

