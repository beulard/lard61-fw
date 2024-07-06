// Query the state of each key and report via USB as a HID.

#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "key_matrix.h"
#include "pico/time.h"
#include "pico/types.h"
#include "tusb.h"
#include "tusb_config.h"

#define LED_PIN PICO_DEFAULT_LED_PIN

// Keymatrix column whose pin is currently high.
// Shared state between the main process and keymatrix_gpio_callback.
volatile uint active_col = 0;

// The array is slightly larger than it need to be: we only have 61 physical
// keys. This is just more convenient for looping over rows and columns.
// Shared state between the main process and keymatrix_gpio_callback.
volatile bool pressed[N_COLS * N_ROWS] = {0};

// Update array of pressed keys
void update_pressed_task();
// Send HID report every 10ms
void hid_task();
// Talk over uart0
void uart_task();
// Blink the led in different ways depending on usb state
void led_task();
// Interrupt callback for a rising edge event on one of the row pins
void keymatrix_gpio_callback(uint gpio, uint32_t event_mask);

int main() {
  // uart will only work on a Pico board, not on the actual lard61
  stdio_init_all();

  printf("RP2040 chip version %d\n", rp2040_chip_version());
  printf("RP2040 rom version %d\n", rp2040_rom_version());

  tud_init(BOARD_TUD_RHPORT);

  setup_keymatrix();

  gpio_init(LED_PIN);
  gpio_set_dir(LED_PIN, GPIO_OUT);

  gpio_set_irq_enabled(row_pins[0], GPIO_IRQ_EDGE_RISE, true);
  gpio_set_irq_callback(&keymatrix_gpio_callback);
  irq_set_enabled(IO_IRQ_BANK0, true);

  while (true) {
    uart_task();

    absolute_time_t update_start = get_absolute_time();
    update_pressed_task();
    int64_t diff = absolute_time_diff_us(update_start, get_absolute_time());
    printf("Time to run update: %lld us\n", diff);

    // Log pressed keys
    for (uint col = 0; col < N_COLS; ++col) {
      for (uint row = 0; row < N_ROWS; ++row) {
        // Check the pressed table and update
        if (pressed[col + get_row_offset(row)]) {
          printf("pressed %d %d\n", col, row);
        }
      }
    }

    hid_task();
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
// IRQ handlers
//--------------------------

void keymatrix_gpio_callback(uint gpio, uint32_t event_mask) {
  // No need to call gpio_acknowledge_irq, it is called automatically
  if (event_mask & GPIO_IRQ_EDGE_RISE) {
    // If we see a rising edge, then the key identified by the active row and
    // column is pressed.
    uint row = get_row(gpio);
    uint row_offset = get_row_offset(row);
    pressed[row_offset + active_col] = true;
  }
}

//--------------------------
// Tasks
//--------------------------

void update_pressed_task() {
  // Turn column on, let irq on rows update the pressed table

  // Reset pressed state to unpressed, let the interrupts set the state high if
  // the key is actually pressed
  memset((void*)pressed, 0, sizeof(pressed));

  // Note active_col is set here and used in the gpio callback to write
  // into the right location of `pressed`.
  for (active_col = 0; active_col < N_COLS; ++active_col) {
    for (uint row = 0; row < N_ROWS; ++row) {
      // Enable the rising edge interrupt for our input pins
      gpio_set_irq_enabled(row_pins[row], GPIO_IRQ_EDGE_RISE, true);
      // Send the high signal in the active column
      gpio_put(col_pins[active_col], true);

      // Any key which is pressed here will trigger a rising edge interrupt on
      // the associated pin, which will call keymatrix_gpio_callback to update
      // the `pressed` table.

      // Disable falling edge irq since we're about to turn the column pin low
      gpio_set_irq_enabled(row_pins[row], GPIO_IRQ_EDGE_RISE, false);
      gpio_put(col_pins[active_col], false);
    }
  }
}

void hid_task() {
  static absolute_time_t start;
  absolute_time_t t = get_absolute_time();

  // Every 10ms, send a report
  if (absolute_time_diff_us(start, t) < 10000) {
    return;
  }
  start = get_absolute_time();

  printf("Hello hid\n");
}

void uart_task() {
  static absolute_time_t start;
  absolute_time_t t = get_absolute_time();

  // Every 2s, print
  if (absolute_time_diff_us(start, t) < 2000000) {
    return;
  }
  start = get_absolute_time();

  printf("alive\n");
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
