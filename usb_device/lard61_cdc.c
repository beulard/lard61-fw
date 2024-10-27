/*
** file: lard61_cdc.c
** author: beulard (Matthias Dubouchet)
** creation date: 11/07/2024
**
** In here we define a command buffer which is filled with data coming from
** the host via the CDC USB interface. The filling is done by tud_cdc_rx_cb.
**
** We also implement a tiny shell where specific strings sent by the host
** are interpreted as instructions, for example to reset the keyboard in
** BOOTSEL mode remotely and allow a subsequent firmware reflash.
*/

#include "lard61_cdc.h"

#include <pico/bootrom.h>
#include <stdarg.h>

#include "class/cdc/cdc_device.h"

//-----------------------------------------------------------------------------
// Static variables
//-----------------------------------------------------------------------------

// Private command buffer with a write pointer
static struct {
  char buffer[LARD61_COMMAND_BUFFER_SIZE];
  // Current write position
  char* write;
} command_buf = {.buffer = {0}, .write = command_buf.buffer};

//-----------------------------------------------------------------------------
// Public API
//-----------------------------------------------------------------------------

void l61_cdc_setup() {
  tud_cdc_set_wanted_char('\r');
}

void l61_printf(const char* fmt, ...) {
  static char buffer[LARD61_PRINTF_BUFFER_SIZE];

  va_list args;
  va_start(args, fmt);
  vsnprintf(buffer, LARD61_PRINTF_BUFFER_SIZE, fmt, args);
  va_end(args);

  tud_cdc_write_str(buffer);
  tud_cdc_write_flush();
}

//-----------------------------------------------------------------------------
// Internal API
//-----------------------------------------------------------------------------

// Display the welcome message
void print_welcome() {
    l61_printf("Hi from lard61 !\n");
    l61_printf("- RP2040 chip version: %d\n", rp2040_chip_version());
    l61_printf("- RP2040 rom version: %d\n\n", rp2040_rom_version());
}

// Display the help message
void print_help() {
    l61_printf("Available commands:\n");
    l61_printf("- hi: greet\n");
    l61_printf("- help: you don't need help\n");
    l61_printf("- flash: restart in bootsel mode\n");
    l61_printf("Magic reflash combination is: Ctrl + Alt + Fn + R\n");
}

// Interpet the data in command_buf as an instruction to perform some action
void process_command_buffer() {
  printf("user entered: '%s'\n", command_buf.buffer);

  if (strcmp(command_buf.buffer, "hi") == 0) {
    l61_printf("hello :)\n");
  } else if (strcmp(command_buf.buffer, "flash") == 0) {
    l61_printf("restarting in bootsel mode...\n");
    reset_usb_boot(1 << PICO_DEFAULT_LED_PIN, 0);
  } else if (strcmp(command_buf.buffer, "help") == 0) {
    print_help();
  } else if (strlen(command_buf.buffer) == 0) {
    // pass
  } else {
    l61_printf("command not found: '%s'\n", command_buf.buffer);
  }

  // Reset command buffer
  command_buf.write = command_buf.buffer;
  command_buf.buffer[0] = '\0';
}

//-----------------------------------------------------------------------------
// USB CDC callbacks
//-----------------------------------------------------------------------------

void tud_cdc_rx_cb(uint8_t itf) {
  (void)itf;

  // Store the new data in the command buffer
  while (tud_cdc_available()) {
    if (command_buf.write < command_buf.buffer + LARD61_COMMAND_BUFFER_SIZE - 1) {
      int32_t c = tud_cdc_read_char();
      if (c == '\r')
        continue;
      (*command_buf.write) = c;
      // tud_cdc_write_char(*command_buf.write);
      command_buf.write++;
      (*command_buf.write) = '\0';
    } else {
      l61_printf("\nToo many chars in command_buf ! Can't process next command\n");
      // Reset the write ptr to the start of the buffer
      command_buf.write = command_buf.buffer;
    }
  }
  l61_printf("\r=> %s", command_buf.buffer);
  tud_cdc_write_flush();
}

void tud_cdc_line_state_cb(uint8_t itf, bool dtr, bool rts) {
  (void)itf;

  if (dtr && rts) {
    // say hi on connection :)
    print_welcome();
    print_help();
  }
}

void tud_cdc_rx_wanted_cb(uint8_t itf, char wanted_char) {
  (void)itf;
  (void)wanted_char;

  tud_cdc_write_char('\n');

  // Consume the contents of the command buffer
  process_command_buffer();
}