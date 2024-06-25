
add_executable(usb_device
        usb_device.c
        usb_descriptors.c
        key_matrix.c
)

# enable output via usb cdc
pico_enable_stdio_uart(usb_device 1)

# Required for tinyusb to find our tusb_config.h
target_include_directories(usb_device PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(usb_device pico_stdlib tinyusb_device)

# create map/bin/hex/uf2 file etc.
pico_add_extra_outputs(usb_device)

