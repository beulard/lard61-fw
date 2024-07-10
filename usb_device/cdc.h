/*
** file: cdc.h
** author: beulard (Matthias Dubouchet)
** creation date: 10/07/2024
*/

#ifndef _CDC_H
#define _CDC_H

#include <stdarg.h>
#include <stdio.h>
#include "tusb.h"

#define LARD61_PRINTF_BUFFER_SIZE 256
#define LARD61_COMMAND_BUFFER_SIZE 32

typedef struct {
  char buffer[LARD61_COMMAND_BUFFER_SIZE];
  // Current write position
  char* write;
} command_buf_t;

command_buf_t command_buf = {.buffer = {0}, .write = command_buf.buffer};

void cdc_setup() {
  tud_cdc_set_wanted_char('\r');
}

void lard61_printf(const char* fmt, ...) {
  static char buffer[LARD61_PRINTF_BUFFER_SIZE];

  va_list args;
  va_start(args, fmt);
  int len = vsnprintf(buffer, LARD61_PRINTF_BUFFER_SIZE, fmt, args);
  va_end(args);

  // printf("len %d\n", len);
  // printf(buffer);

  tud_cdc_write_str(buffer);
  tud_cdc_write_flush();
}

//--------------------------
// USB CDC callbacks
//--------------------------

// Invoked when received new data
void tud_cdc_rx_cb(uint8_t itf) {
  (void)itf;

  // printf("cdc_rx_cb\n");
  // Store the new data in the command buffer
  while (tud_cdc_available()) {
    if (command_buf.write < command_buf.buffer + LARD61_COMMAND_BUFFER_SIZE) {
      int32_t c = tud_cdc_read_char();
      if (c == '\r') continue;
      (*command_buf.write) = c;
      // tud_cdc_write_char(*command_buf.write);
      command_buf.write++;
      (*command_buf.write) = '\0';
    } else {
      command_buf.write = command_buf.buffer;
      printf("\nToo many chars in command_buf ! Can't process next command\n");
    }
  }
  lard61_printf("\r=> %s", command_buf.buffer);
  tud_cdc_write_flush();
  // printf(command_buf.buffer);
  // printf("\n");
}

// Invoked when cdc when line state changed e.g connected/disconnected
void tud_cdc_line_state_cb(uint8_t itf, bool dtr, bool rts) {
  (void)itf;

  if (dtr && rts) {
    // say hi on connection :)
    lard61_printf("Hi from lard61 !\n");
    lard61_printf("- RP2040 chip version: %d\n", rp2040_chip_version());
    lard61_printf("- RP2040 rom version: %d\n", rp2040_rom_version());
    lard61_printf("\n");
    lard61_printf("Available commands:\n");
    lard61_printf("- hi: greet\n");
    lard61_printf("- help: you don't need help\n");
    lard61_printf("- flash: restart in bootsel mode\n");
  }
}

void process_command_buffer() {
  printf("user entered: '%s'\n", command_buf.buffer);

  if (strcmp(command_buf.buffer, "hi") == 0) {
    lard61_printf("hello :)\n");
  } else if (strcmp(command_buf.buffer, "flash") == 0) {
    lard61_printf("restarting in bootsel mode...\n");
    // TODO(mdu)
  } else {
    lard61_printf("command not found: '%s'\n", command_buf.buffer);
  }
  
  // Reset command buffer
  command_buf.write = command_buf.buffer;
  command_buf.buffer[0] = '\0';
}

// Wait for a carriage return to process the command buffer
void tud_cdc_rx_wanted_cb(uint8_t itf, char wanted_char) {
  (void)itf;

  tud_cdc_write_char('\n');

  // Consume the contents of the command buffer
  process_command_buffer();
}

#endif /* _CDC_H */