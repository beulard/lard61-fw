/*
** file: usb_device.c
** author: beulard (Matthias Dubouchet)
** creation date: 11/07/2024
**
** Main file for the lard61 firmware.
** Contains the setup and main loop functions, as well as
** "task" functions to handle HID reporting and LED blinking.
**
** A couple of simple USB callbacks are also defined here.
*/

#include <stdint.h>
#include "class/hid/hid_device.h"
#include "device/usbd.h"
#include "hardware/gpio.h"
#include "lard61_cdc.h"
#include "lard61_keycodes.h"
#include "lard61_keymatrix.h"
#include "pico/bootrom.h"
#include "pico/stdio.h"
#include "pico/time.h"
#include "pico/types.h"
#include "tusb_config.h"

#define LED_PIN PICO_DEFAULT_LED_PIN

// Poll the keymatrix every 1ms and send a HID report
void hid_task();
// Blink the led in different ways depending on usb state
void led_task();

int main() {
  // uart will only work on a Pico board, not on the actual lard61
  stdio_init_all();

  tud_init(BOARD_TUD_RHPORT);

  l61_cdc_setup();
  l61_keymatrix_setup();

  gpio_init(LED_PIN);
  gpio_set_dir(LED_PIN, GPIO_OUT);

  while (true) {
    tud_task();
    l61_keymatrix_update();
    hid_task();
    led_task();
  }
}

//-----------------------------------------------------------------------------
// Blink parameters
//-----------------------------------------------------------------------------

#define BLINK_MOUNTED 250
#define BLINK_UNMOUNTED 1000
#define BLINK_SUSPENDED 2500

uint blink_interval_ms = BLINK_SUSPENDED;

//-----------------------------------------------------------------------------
// Tasks
//-----------------------------------------------------------------------------

void hid_task() {
  static absolute_time_t start;
  absolute_time_t t = get_absolute_time();

  // As soon as HID interface is ready, send a report
  if (!tud_hid_ready()) {
    return;
  } else {
    // l61_printf("hid ready after %lld us\n", absolute_time_diff_us(start, t));
    start = get_absolute_time();
    (void)t;
    (void)start;
  }

  // l61_keymatrix_report();

  // Value of next_idx after the last report
  static uint prev_pressed_count = 0;
  // Pressed key keycodes
  uint8_t keycode[6] = {0};
  // To index into `keycode`
  uint next_idx = 0;

  // Check for the magic reflash combination:
  // If user presses Ctrl + Alt + Fn + R, reboot in usb flash mode
  if (l61_keymatrix_is_key_pressed(L61_KEY_LEFT_CONTROL) &&
      l61_keymatrix_is_key_pressed(L61_KEY_LEFT_ALT) &&
      l61_keymatrix_is_key_pressed(L61_KEY_FN) &&
      l61_keymatrix_is_key_pressed(L61_KEY_R)) {
    reset_usb_boot(1 << PICO_DEFAULT_LED_PIN, 0);
  }

  // Transform the "pressed" table from l61_keymatrix into
  // the hid 6-byte keycode report
  for (uint i = 0; i < N_ROWS * N_COLS; ++i) {
    // Avoid overflowing the buffer if too many keys are pressed
    if (next_idx >= 6)
      break;

    if (l61_keymatrix_is_key_pressed(i)) {
      // l61_printf("key %d pressed\n", i);
      if (l61_keymatrix_is_fn_key_pressed()) {
        keycode[next_idx++] = l61_hid_keycode_fn[i];
      } else {
        keycode[next_idx++] = l61_hid_keycode[i];
      }
    }

    // Do not report more than one additional key compared to the previous
    // report. This prevents multiple keys being repeated.
    //
    // Without this, if the user presses and holds Q and W simultaneously,
    // the host will repeat both q and w, yielding "qwqwqwqwqw...". The expected
    // behaviour is to repeat the last key that was pressed, so in case of
    // exactly simultaneous keypresses, give priority to the lowest key index.
    if (next_idx >= prev_pressed_count + 1)
      break;
  }

  prev_pressed_count = next_idx;

  if (next_idx == 0) {
    tud_hid_keyboard_report(0, 0, NULL);
  } else {
    // l61_printf("sending keycodes: %d %d %d %d %d %d\n", keycode[0],
    //            keycode[1], keycode[2], keycode[3], keycode[4], keycode[5]);
    tud_hid_keyboard_report(0, 0, keycode);
  }
}

void led_task() {
  static absolute_time_t start;
  static bool led_state = false;

  absolute_time_t t = get_absolute_time();

  if (absolute_time_diff_us(start, t) < blink_interval_ms * 1000) {
    return;
  }
  start = get_absolute_time();

  gpio_put(LED_PIN, led_state);
  led_state = !led_state;
}

//-----------------------------------------------------------------------------
// USB state callbacks
//-----------------------------------------------------------------------------

// USB bus is mounted (configured)
void tud_mount_cb() {
  blink_interval_ms = BLINK_MOUNTED;
}

// USB bus is unmounted
void tud_umount_cb() {
  blink_interval_ms = BLINK_UNMOUNTED;
}

// USB bus is suspended
void tud_suspend_cb(bool remote_wakeup_en) {
  (void)remote_wakeup_en;
  blink_interval_ms = BLINK_SUSPENDED;
}

// USB bus is resumed
void tud_resume_cb() {
  blink_interval_ms = tud_mounted() ? BLINK_MOUNTED : BLINK_UNMOUNTED;
}

//--------------------------------------------------------------------+
// USB HID callbacks
//--------------------------------------------------------------------+

// Invoked when received GET_REPORT control request
uint16_t tud_hid_get_report_cb(uint8_t itf,
                               uint8_t report_id,
                               hid_report_type_t report_type,
                               uint8_t* buffer,
                               uint16_t reqlen) {
  (void)itf;
  (void)report_id;
  (void)report_type;
  (void)buffer;
  (void)reqlen;

  return 0;
}

// Invoked when received SET_REPORT control request or
// received data on OUT endpoint ( Report ID = 0, Type = 0 )
void tud_hid_set_report_cb(uint8_t itf,
                           uint8_t report_id,
                           hid_report_type_t report_type,
                           uint8_t const* buffer,
                           uint16_t bufsize) {
  // This example doesn't use multiple report and report ID
  (void)itf;
  (void)report_id;

  if (report_type == HID_REPORT_TYPE_OUTPUT) {
    // bufsize should be (at least) 1
    if (bufsize < 1)
      return;

    uint8_t const kbd_leds = buffer[0];

    if (kbd_leds & KEYBOARD_LED_CAPSLOCK) {
      l61_printf("Capslock on !\n");
    } else {
      l61_printf("Capslock off !\n");
    }
  }

  // echo back anything we received from host
  tud_hid_report(0, buffer, bufsize);
}
