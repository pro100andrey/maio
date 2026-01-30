# Generated Cmake Pico project file

cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Initialise pico_sdk from installed location
# (note this can come from environment, CMake cache etc)

# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.2.0)
set(toolchainVersion 14_2_Rel1)
set(picotoolVersion 2.2.0-a4)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================
set(PICO_BOARD pico2_w CACHE STRING "Board type")

# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)

project(maio C CXX ASM)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

# ---- Project Configuration ----
# Edit values below to configure project features

# Debug & Development Features
set(DEBUG_VERBOSE OFF)
set(ENABLE_ASSERTS ON)

# Application Features
set(ENABLE_WIFI OFF)  # WiFi disabled
set(ENABLE_ENCODER ON)
set(ENABLE_LED ON)

# Performance Tuning
set(EVENT_QUEUE_SIZE 10)
set(SYSTEM_TICK_MS 10)

# Add executable. Default name is the project name, version 0.1

add_executable(maio 
    src/main.c
    src/core/event_manager.c
    src/core/strategy.c
    src/strategies/idle_strategy.c
    src/managers/encoder_manager.c
    drivers/encoder/encoder.c
)

pico_set_program_name(maio "maio")
pico_set_program_version(maio "0.1")

# no_flash means the target is to run from RAM
pico_set_binary_type(maio no_flash)

# Modify the below lines to enable/disable output over UART/USB
pico_enable_stdio_uart(maio 1)
pico_enable_stdio_usb(maio 0)

# Apply compile definitions
target_compile_definitions(maio PRIVATE
    EVENT_QUEUE_SIZE=${EVENT_QUEUE_SIZE}
    SYSTEM_TICK_MS=${SYSTEM_TICK_MS}
    $<$<BOOL:${DEBUG_VERBOSE}>:DEBUG_VERBOSE>
    $<$<BOOL:${ENABLE_ASSERTS}>:ENABLE_ASSERTS>
    $<$<BOOL:${ENABLE_WIFI}>:ENABLE_WIFI>
    $<$<BOOL:${ENABLE_ENCODER}>:ENABLE_ENCODER>
    $<$<BOOL:${ENABLE_LED}>:ENABLE_LED>
)

# Add the standard library to the build
target_link_libraries(maio
        pico_stdlib
        hardware_gpio
        hardware_sync
        hardware_clocks)

# Add the standard include files to the build
target_include_directories(maio PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}
        ${CMAKE_CURRENT_LIST_DIR}/src/shared
        ${CMAKE_CURRENT_LIST_DIR}/drivers
)

pico_add_extra_outputs(maio)

