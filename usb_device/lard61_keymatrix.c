/*
** file: lard61_keymatrix.c
** author: beulard (Matthias Dubouchet)
** creation date: 11/07/2024
*/

#include "lard61_keymatrix.h"
#include <stdio.h>
#include <string.h>
#include "hardware/gpio.h"
#include "lard61_cdc.h"

//-----------------------------------------------------------------------------
// Static variables
//-----------------------------------------------------------------------------

// Keymatrix column whose pin is currently high.
// Shared state between the main process and keymatrix_gpio_callback.
volatile uint active_col = 0;

// The array is slightly larger than it need to be: we only have 61 physical
// keys. This is just more convenient for looping over rows and columns.
// Shared state between the main process and keymatrix_gpio_callback.
volatile bool pressed[N_COLS * N_ROWS] = {0};

// Number of keys per row in the keymatrix
static const uint n_keys_in_row[N_ROWS] = {14, 14, 13, 12, 8};

// GPIO pins for each row
static const uint row_pins[N_ROWS] = {
    19,  // row0
    20,  // row1
    21,  // row2
    22,  // row3
    18,  // row4
};

// GPIO pins for each column
// TODO(mdu) clean up
static const uint col_pins[N_COLS] = {
    23,  // col0
#ifdef LARD61
    25,  // col1
#elif defined RASPBERRYPI_PICO
    // Used for LED
    23,  // col1
#else
    #error Unknown board
#endif
    26,  // col2
    24,  // col3
    27,  // col4
    28,  // col5
    29,  // col6
#ifdef LARD61
    0,   // col7
    1,   // col8
#elif defined RASPBERRYPI_PICO
    // Also used for uart comm on dev Pico => use col6 pin
    29,  // 0,   // col7
    29,  // 1,   // col8
#else
    #error Unknown board
#endif
    2,   // col9
    3,   // col10
    4,   // col11
    5,   // col12
    6,   // col12
};


//-----------------------------------------------------------------------------
// Public API
//-----------------------------------------------------------------------------

void l61_keymatrix_setup() {
  // Set all row pins as input
  // printf("Row pins\n");
  for (uint i = 0; i < N_ROWS; ++i) {
    // printf("%d => %d\n", i, row_pins[i]);
    gpio_init(row_pins[i]);
    gpio_set_dir(row_pins[i], GPIO_IN);
  }

  // Set all column pins as output
  // printf("Col pins\n");
  for (uint i = 0; i < N_COLS; ++i) {
    // printf("%d => %d\n", i, col_pins[i]);
    gpio_init(col_pins[i]);
    gpio_set_dir(col_pins[i], GPIO_OUT);
  }

  printf("Key matrix pins configured\n");

  // Enable rising edge interrupt on all row pins
  gpio_set_irq_enabled(row_pins[0], GPIO_IRQ_EDGE_RISE, true);
  gpio_set_irq_callback(&l61_keymatrix_gpio_callback);
  irq_set_enabled(IO_IRQ_BANK0, true);

  printf("Key matrix interrupts OK\n");
}

uint l61_keymatrix_get_row(uint gpio) {
  switch (gpio) {
    case 19:
      return 0;
    case 20:
      return 1;
    case 21:
      return 2;
    case 22:
      return 3;
    case 18:
      return 4;
    default:
      return (uint)(-1);
  }
}

uint l61_keymatrix_get_row_offset(uint row) {
  uint offset = 0;
  for (uint i = 0; i < row; ++i) {
    offset += n_keys_in_row[i];
  }
  return offset;
}

// TODO(mdu) rename once we know what hid report is expecting
void l61_keymatrix_update() {
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

void l61_keymatrix_report() {
  // Log pressed keys
  for (uint col = 0; col < N_COLS; ++col) {
    for (uint row = 0; row < N_ROWS; ++row) {
      // Check the pressed table and update
      if (pressed[col + l61_keymatrix_get_row_offset(row)]) {
        l61_printf("pressed %d %d\n", col, row);
      }
    }
  }
}

//-----------------------------------------------------------------------------
// IRQ callbacks
//-----------------------------------------------------------------------------

void l61_keymatrix_gpio_callback(uint gpio, uint32_t event_mask) {
  // No need to call gpio_acknowledge_irq, it is called automatically
  if (event_mask & GPIO_IRQ_EDGE_RISE) {
    // If we see a rising edge, then the key identified by the active row and
    // column is pressed.
    uint row = l61_keymatrix_get_row(gpio);
    uint row_offset = l61_keymatrix_get_row_offset(row);
    pressed[row_offset + active_col] = true;
  }
}
