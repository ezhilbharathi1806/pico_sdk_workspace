cmake_minimum_required(VERSION 3.12)

#include(pico_sdk_import.cmake)
include($ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake)

project(pico-multicore)

pico_sdk_init()

add_executable(${PROJECT_NAME}
        basic_multicore.c
)

target_link_libraries(${PROJECT_NAME}
        pico_multicore
        pico_stdlib
        hardware_adc
)

pico_enable_stdio_usb(${PROJECT_NAME} 1)
pico_enable_stdio_uart(${PROJECT_NAME} 0)

pico_add_extra_outputs(${PROJECT_NAME})