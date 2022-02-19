cmake_minimum_required(VERSION 3.13)

include(pico_sdk_import.cmake)

project(drive_module C CXX ASM)

pico_sdk_init()

file( GLOB source_files
                "src/*.c"
                "src/*.cpp"
)

add_executable(drive ${source_files})

target_link_libraries(drive pico_bootrom pico_multicore hardware_gpio hardware_i2c hardware_pwm pico_stdlib)
target_include_directories(drive PRIVATE "./include")

pico_enable_stdio_usb(drive true)
pico_add_extra_outputs(drive)