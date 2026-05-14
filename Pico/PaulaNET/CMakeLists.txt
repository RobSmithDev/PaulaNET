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
set(PICO_BOARD pico_w CACHE STRING "Board type")

# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)

project(PaulaNET C CXX ASM)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

# Add executable. Default name is the project name, version 0.1

add_executable(PaulaNET PaulaNET.cpp rleCompression.cpp trackBuilder.cpp DiskImage.cpp)

pico_set_program_name(PaulaNET "PaulaNET")
pico_set_program_version(PaulaNET "0.1")

# Generate PIO header
pico_generate_pio_header(PaulaNET ${CMAKE_CURRENT_LIST_DIR}/floppytx.pio)
 
# Modify the below lines to enable/disable output over UART/USB
pico_enable_stdio_uart(PaulaNET 1)
pico_enable_stdio_usb(PaulaNET 0)

# Add the standard include files to the build
target_include_directories(PaulaNET PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}
)

target_compile_definitions(PaulaNET PRIVATE
    LWIP_DHCP=0
)

# Add any user requested libraries
target_link_libraries(PaulaNET 
        pico_stdlib
        hardware_dma
        hardware_pio
        pico_multicore
        hardware_flash 
        pico_cyw43_arch_lwip_poll
        )

pico_add_extra_outputs(PaulaNET)

