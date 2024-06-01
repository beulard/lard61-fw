cmake_minimum_required(VERSION 3.13)

include(pico_sdk_import.cmake)

project(lard61-blink C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
# Generate compile_commands.json for clangd
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

pico_sdk_init()

add_executable(blink
        blink.c
)

pico_enable_stdio_uart(blink 0)
pico_enable_stdio_usb(blink 0)

# pull in common dependencies
target_link_libraries(blink pico_stdlib)

# create map/bin/hex file etc.
pico_add_extra_outputs(blink)

add_custom_target(deploy
  COMMAND sudo openocd -f interface/cmsis-dap.cfg -f target/rp2040.cfg -c "adapter speed 5000" -c "program blink.elf verify reset exit"
  DEPENDS blink
)

add_custom_target(reset
  COMMAND sudo openocd -f interface/cmsis-dap.cfg -f target/rp2040.cfg -c "init; reset; exit"
)

add_custom_target(debug-server
  COMMAND sudo openocd -f interface/cmsis-dap.cfg -f target/rp2040.cfg
)

add_custom_target(debug
  COMMAND gdb-multiarch -x init.gdb
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
)
