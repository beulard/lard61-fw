/**
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "boards/pico.h"
#include "hardware/gpio.h"
#include "hardware/uart.h"
#include "pico/time.h"
#include "pico/types.h"
#include "tusb_config.h"
#include "tusb.h"

void uart_task();
void led_task();

int main() {
  uart_init(uart0, 115200);

  tud_init(BOARD_TUD_RHPORT);

  gpio_set_function(0, GPIO_FUNC_UART);
  gpio_set_function(1, GPIO_FUNC_UART);

  gpio_init(PICO_DEFAULT_LED_PIN);
  gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);

  while (true) {
    uart_task();
    // TODO(mdu) do something different depending on the USB state
    // Connected, sleeping, etc.
    led_task();
    tud_task();
  }
}

void uart_task() {
  static absolute_time_t start;

  absolute_time_t t = get_absolute_time();

  // Every 1s, print
  if (absolute_time_diff_us(start, t) < 1000000) {
    return;
  }
  start = get_absolute_time();

  uart_puts(uart0, "Hello, saucisse!\r\n");
}

void led_task() {
  static absolute_time_t start;
  static bool led_state = false;

  absolute_time_t t = get_absolute_time();

  // Every 250ms, toggle
  if (absolute_time_diff_us(start, t) < 125000) {
    return;
  }
  start = get_absolute_time();

  gpio_put(PICO_DEFAULT_LED_PIN, led_state);
  led_state = !led_state;
}

//--------------------------------------------------------------------+
// USB HID
//--------------------------------------------------------------------+

// Invoked when received GET_REPORT control request
// Application must fill buffer report's content and return its length.
// Return zero will cause the stack to STALL request
uint16_t tud_hid_get_report_cb(uint8_t itf, uint8_t report_id, hid_report_type_t report_type, uint8_t* buffer, uint16_t reqlen)
{
  // TODO not Implemented
  (void) itf;
  (void) report_id;
  (void) report_type;
  (void) buffer;
  (void) reqlen;

  return 0;
}

// Invoked when received SET_REPORT control request or
// received data on OUT endpoint ( Report ID = 0, Type = 0 )
void tud_hid_set_report_cb(uint8_t itf, uint8_t report_id, hid_report_type_t report_type, uint8_t const* buffer, uint16_t bufsize)
{
  // This example doesn't use multiple report and report ID
  (void) itf;
  (void) report_id;
  (void) report_type;

  // echo back anything we received from host
  tud_hid_report(0, buffer, bufsize);
}
