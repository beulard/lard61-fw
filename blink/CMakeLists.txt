
add_executable(blink
        blink.c
)

pico_enable_stdio_uart(blink 0)
pico_enable_stdio_usb(blink 0)

# pull in common dependencies
target_link_libraries(blink pico_stdlib)

# create map/bin/hex file etc.
pico_add_extra_outputs(blink)

