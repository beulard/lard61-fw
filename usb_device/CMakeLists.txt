
add_executable(usb_device
        usb_device.c
        usb_descriptors.c
        lard61_keymatrix.c
        lard61_cdc.c
)

target_compile_options(usb_device PUBLIC -Wall -Wextra -fdiagnostics-color=always)

if (${PICO_BOARD} STREQUAL "lard61")
 message("Configuring for lard61 -> disable usb and uart stdio")
 pico_enable_stdio_uart(usb_device 0)
 pico_enable_stdio_usb(usb_device 0)
else ()
 message("Configuring for pico -> enable uart stdio")
 pico_enable_stdio_uart(usb_device 1)
 pico_enable_stdio_usb(usb_device 0)
endif()

# Required for tinyusb to find our tusb_config.h
target_include_directories(usb_device PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(usb_device pico_stdlib tinyusb_device)

# create map/bin/hex/uf2 file etc.
pico_add_extra_outputs(usb_device)

add_custom_target(deploy
 COMMAND openocd -f interface/cmsis-dap.cfg -f target/rp2040.cfg -c "adapter speed 5000" -c "program usb_device.elf verify reset exit"
 DEPENDS usb_device
)
