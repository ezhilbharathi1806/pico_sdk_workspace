cmake_minimum_required(VERSION 3.12)                # Set minimum required version of CMake

include($ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake)     # Include build functions from Pico SDK

project(pico_i2c)                   # Set name of project (as PROJECT_NAME) and C/C   standards
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

pico_sdk_init()

add_executable(${PROJECT_NAME}      # Tell CMake where to find the executable source file
    mpu6050_i2c.c
)

target_link_libraries(${PROJECT_NAME}       # Link to pico_stdlib 
    pico_stdlib
    hardware_i2c
)

pico_enable_stdio_usb(${PROJECT_NAME} 1)    # Enable usb output, disable uart output
pico_enable_stdio_uart(${PROJECT_NAME} 0)

pico_add_extra_outputs(${PROJECT_NAME})     # Create map/bin/hex/uf2 files