cmake_minimum_required(VERSION 3.13)

# Pull in SDK (must be before project)
include(pico_sdk_import.cmake)

project(lard61-fw C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# Generate compile_commands.json for clangd
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Initialize the SDK
pico_sdk_init()

add_subdirectory(blink)
add_subdirectory(serial)

#add_custom_target(deploy
#  COMMAND sudo openocd -f interface/cmsis-dap.cfg -f target/rp2040.cfg -c "adapter speed 5000" -c "program blink.elf verify reset exit"
#  DEPENDS blink
#)

add_custom_target(reset
  COMMAND sudo openocd -f interface/cmsis-dap.cfg -f target/rp2040.cfg -c "init; reset; exit"
)

add_custom_target(debug-server
  COMMAND sudo openocd -f interface/cmsis-dap.cfg -f target/rp2040.cfg
)

#add_custom_target(debug
#  COMMAND gdb-multiarch -x init.gdb
#  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
#)