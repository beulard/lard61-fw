
add_executable(usb_output
        usb_output.c
)

# pull in common dependencies
target_link_libraries(usb_output pico_stdlib)

# enable output via usb
pico_enable_stdio_usb(usb_output 1)

# create map/bin/hex/uf2 file etc.
pico_add_extra_outputs(usb_output)

