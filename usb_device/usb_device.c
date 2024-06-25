// Appear as a USB HID device to the USB host

#include "boards/pico.h"
#include "hardware/gpio.h"
#include "hardware/uart.h"
#include "pico/time.h"
#include "pico/types.h"
#include "tusb_config.h"
#include "tusb.h"

#define LED_PIN PICO_DEFAULT_LED_PIN

// Talk over uart0
void uart_task();
// Blink the led in different ways depending on usb state
void led_task();

int main() {
  // uart will only work on a Pico board, not on the actual lard61
  uart_init(uart0, 115200);

  tud_init(BOARD_TUD_RHPORT);

  gpio_set_function(0, GPIO_FUNC_UART);
  gpio_set_function(1, GPIO_FUNC_UART);

  gpio_init(LED_PIN);
  gpio_set_dir(LED_PIN, GPIO_OUT);

  while (true) {
    uart_task();
    led_task();
    tud_task();
  }
}


//--------------------------
// Blink parameters
//--------------------------
#define BLINK_MOUNTED 250
#define BLINK_UNMOUNTED 1000
#define BLINK_SUSPENDED 2500

uint blink_interval_ms = BLINK_SUSPENDED;

//--------------------------
// USB callbacks
//--------------------------

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

//--------------------------
// Tasks
//--------------------------

void uart_task() {
  static absolute_time_t start;

  absolute_time_t t = get_absolute_time();

  // Every 1s, print
  if (absolute_time_diff_us(start, t) < 1000000) {
    return;
  }
  start = get_absolute_time();

  uart_puts(uart0, "Hello from pico!\r\n");
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
