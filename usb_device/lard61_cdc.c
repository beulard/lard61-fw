/*
** file: lard61_cdc.c
** author: beulard (Matthias Dubouchet)
** creation date: 11/07/2024
*/

#include "lard61_cdc.h"

// Private command buffer with a write pointer
struct {
  char buffer[LARD61_COMMAND_BUFFER_SIZE];
  // Current write position
  char* write;
} command_buf = {.buffer = {0}, .write = command_buf.buffer};

//--------------------------
// Public API
//--------------------------

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

//--------------------------
// Internal API
//--------------------------

// Use the data in command_buf
void process_command_buffer() {
  printf("user entered: '%s'\n", command_buf.buffer);

  if (strcmp(command_buf.buffer, "hi") == 0) {
    l61_printf("hello :)\n");
  } else if (strcmp(command_buf.buffer, "flash") == 0) {
    l61_printf("restarting in bootsel mode...\n");
    reset_usb_boot(1 << PICO_DEFAULT_LED_PIN, 0);
  } else if (strlen(command_buf.buffer) == 0) {
    // pass
  } else {
    l61_printf("command not found: '%s'\n", command_buf.buffer);
  }

  // Reset command buffer
  command_buf.write = command_buf.buffer;
  command_buf.buffer[0] = '\0';
}

//--------------------------
// USB CDC callbacks
//--------------------------

void tud_cdc_rx_cb(uint8_t itf) {
  (void)itf;

  // Store the new data in the command buffer
  while (tud_cdc_available()) {
    if (command_buf.write < command_buf.buffer + LARD61_COMMAND_BUFFER_SIZE) {
      int32_t c = tud_cdc_read_char();
      if (c == '\r')
        continue;
      (*command_buf.write) = c;
      // tud_cdc_write_char(*command_buf.write);
      command_buf.write++;
      (*command_buf.write) = '\0';
    } else {
      command_buf.write = command_buf.buffer;
      printf("\nToo many chars in command_buf ! Can't process next command\n");
    }
  }
  l61_printf("\r=> %s", command_buf.buffer);
  tud_cdc_write_flush();
}

void tud_cdc_line_state_cb(uint8_t itf, bool dtr, bool rts) {
  (void)itf;

  if (dtr && rts) {
    // say hi on connection :)
    l61_printf("Hi from lard61 !\n");
    l61_printf("- RP2040 chip version: %d\n", rp2040_chip_version());
    l61_printf("- RP2040 rom version: %d\n", rp2040_rom_version());
    l61_printf("\n");
    l61_printf("Available commands:\n");
    l61_printf("- hi: greet\n");
    l61_printf("- help: you don't need help\n");
    l61_printf("- flash: restart in bootsel mode\n");
  }
}

void tud_cdc_rx_wanted_cb(uint8_t itf, char wanted_char) {
  (void)itf;
  (void)wanted_char;

  tud_cdc_write_char('\n');

  // Consume the contents of the command buffer
  process_command_buffer();
}