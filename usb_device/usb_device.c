// Query the state of each key and report via USB as a HID.

#include "hardware/gpio.h"
#include "key_matrix.h"
#include "pico/stdlib.h"
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

// TODO(mdu) can use rx cb to fill ?
// TODO(mdu) make it a ring buffer
char command_buffer[256];

// Update array of pressed keys
void update_pressed_task();
// Send HID report every 10ms
void hid_task();
// Talk over uart0
void uart_task();
// Blink the led in different ways depending on usb state
void led_task();
// Send data as a serial device
void cdc_task();
// Interrupt callback for a rising edge event on one of the row pins
void keymatrix_gpio_callback(uint gpio, uint32_t event_mask);

int main() {
  // uart will only work on a Pico board, not on the actual lard61
  stdio_init_all();

  printf("RP2040 chip version %d\n", rp2040_chip_version());
  printf("RP2040 rom version %d\n", rp2040_rom_version());

  tud_init(BOARD_TUD_RHPORT);
  tud_cdc_set_wanted_char('\r');

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
    // printf("Time to run update: %lld us\n", diff);

    // Log pressed keys
    for (uint col = 0; col < N_COLS; ++col) {
      for (uint row = 0; row < N_ROWS; ++row) {
        // Check the pressed table and update
        if (pressed[col + get_row_offset(row)]) {
          printf("pressed %d %d\n", col, row);
        }
      }
    }

    tud_task();
    // hid_task();
    led_task();
    cdc_task();
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

  // TODO(mdu) poll keys and send report
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

void cdc_task() {
  // printf("cdc task\n");
  // connected() check for DTR bit
  // Most but not all terminal client set this when making connection
  if (tud_cdc_connected()) {
    if (tud_cdc_available()) {
      printf("read\n");

      uint8_t buf[64];
      uint sz = tud_cdc_read(buf, sizeof(buf));

      tud_cdc_write(buf, sz);
      tud_cdc_write_flush();
    }
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
// USB CDC
//--------------------------

// Invoked when cdc when line state changed e.g connected/disconnected
void tud_cdc_line_state_cb(uint8_t itf, bool dtr, bool rts) {
  (void)itf;

  if (dtr && rts) {
    // say hi on connection :)
    tud_cdc_write_str("Hi from lard61 !\r\n");
    tud_cdc_write_flush();
  }
}

// Wait for a specific character
void tud_cdc_rx_wanted_cb(uint8_t itf, char wanted_char) {
  (void)itf;

  tud_cdc_write_char('\n');
  tud_cdc_write_flush();

  // Consume the contents of the command buffer
  // TODO(mdu)
  // process_command_buffer();
}

//--------------------------------------------------------------------+
// USB HID
//--------------------------------------------------------------------+

// Invoked when received GET_REPORT control request
// Application must fill buffer report's content and return its length.
// Return zero will cause the stack to STALL request
uint16_t tud_hid_get_report_cb(uint8_t itf,
                               uint8_t report_id,
                               hid_report_type_t report_type,
                               uint8_t* buffer,
                               uint16_t reqlen) {
  // TODO not Implemented
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
  (void)report_type;

  // echo back anything we received from host
  tud_hid_report(0, buffer, bufsize);
}
