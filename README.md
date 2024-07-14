# lard61-fw

Firmware for the [lard61](https://github.com/beulard/lard61) keyboard.

Directories `blink` and `usb_output` contain boilerplate projects I used to test my PCB initially.

The main keyboard firmware is located under `usb_device`.

## Features:

The `usb_device` project is currently a complete USB HID keyboard firmware for
the lard61.

I've also integrated a CDC USB interface with a tiny custom shell, to
allow me to remotely interact with the keyboard and e.g. go into BOOTSEL mode
by sending "flash" through a serial terminal.

When I designed the PCB, I naively omitted connections to the RP2040's debug
pins (SWD, SWCLK). This means I can't remotely flash firmware during
development and try stuff out quickly. To reprogram the firmware before the
CDC USB interface was added, I had to physically short the BOOTSEL pins on the
PCB.
