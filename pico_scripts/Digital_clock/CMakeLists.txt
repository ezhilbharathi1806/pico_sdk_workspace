cmake_minimum_required(VERSION 3.13)

# Project name
set(ProjectName digital_clock)

# Include Pico SDK
include(pico_sdk_import.cmake)

# Define project
project(${ProjectName} C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# Initialize Pico SDK
pico_sdk_init()

# Add executable
add_executable(${ProjectName}
    src/main.c
    src/ds1307_rtc.c
    src/ssd1306.c  # Include SSD1306 library source
)

# Include directories
target_include_directories(${ProjectName} PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}
    ${CMAKE_CURRENT_LIST_DIR}/inc  # Path to ssd1306.h and font.h
)

# Link libraries
target_link_libraries(${ProjectName}
    pico_stdlib
    hardware_i2c  # Required for I2C communication
)

# Enable USB output
pico_enable_stdio_usb(${ProjectName} 1)
pico_enable_stdio_uart(${ProjectName} 0)

# Create map/bin/hex/uf2 files
pico_add_extra_outputs(${ProjectName})
