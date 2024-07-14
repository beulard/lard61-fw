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
// Shared state between the main process and l61_keymatrix_gpio_callback.
volatile uint active_col = 0;

// The array is slightly larger than it need to be: we only have 61 physical
// keys. This is just more convenient for looping over rows and columns.
// Shared state between the main process and l61_keymatrix_gpio_callback.
volatile bool pressed[N_COLS * N_ROWS] = {0};

// Index of the function key in `pressed`
#define L61_FN_KEY 63

// Number of keys per row in the keymatrix
static const uint n_keys_in_row[N_ROWS] = {14, 14, 13, 12, 8};

// GPIO pins for each row
static const uint row_pin[N_ROWS] = {
    19,  // row0
    20,  // row1
    21,  // row2
    22,  // row3
    18,  // row4
};

// Used as a bitmask for the return value of gpio_get_all(),
// to determine if all row pins are low.
static const uint row_pin_mask = (1 << row_pin[0]) | (1 << row_pin[1]) |
                                 (1 << row_pin[2]) | (1 << row_pin[3]) |
                                 (1 << row_pin[4]);

#ifndef LARD61
// Change GPIO mapping on Pico to avoid interfering
// with LED and UART pins.
static const uint col_pin[N_COLS] = {
    23,  // col0
    // Pin 25 is the LED pin on Pico
    // => change to same pin as col0
    23,  // col1
    26,  // col2
    24,  // col3
    27,  // col4
    28,  // col5
    29,  // col6
    // Pins 0, 1 are used for uart comm on dev Pico
    // => change to same pin as col6
    29,  // col7
    29,  // col8
}
#else
// GPIO pins for each column
static const uint col_pin[N_COLS] = {
    23,  // col0
    25,  // col1
    26,  // col2
    24,  // col3
    27,  // col4
    28,  // col5
    29,  // col6
    0,   // col7
    1,   // col8
    2,   // col9
    3,   // col10
    4,   // col11
    5,   // col12
    6,   // col12
};
#endif

//-----------------------------------------------------------------------------
// Internal API
//-----------------------------------------------------------------------------

// Interrupt callback for a rising edge event on one of the row pins
void l61_keymatrix_gpio_callback(uint gpio, uint32_t event_mask);

//-----------------------------------------------------------------------------
// Public API
//-----------------------------------------------------------------------------

void l61_keymatrix_setup() {
  // Set all row pins as input
  for (uint row = 0; row < N_ROWS; ++row) {
    gpio_init(row_pin[row]);
    gpio_set_dir(row_pin[row], GPIO_IN);
  }

  // Set all column pins as output
  for (uint col = 0; col < N_COLS; ++col) {
    gpio_init(col_pin[col]);
    gpio_set_dir(col_pin[col], GPIO_OUT);
  }

  printf("Key matrix pins configured\n");

  // Enable rising edge interrupt on all row pins
  for (uint row = 0; row < N_ROWS; ++row) {
    gpio_set_irq_enabled(row_pin[row], GPIO_IRQ_EDGE_RISE, true);
  }
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

void l61_keymatrix_update() {
  // Turn column on, let irq on rows update the pressed table

  // Reset pressed state to unpressed, let the interrupts set the state high if
  // the key is actually pressed
  for (uint i = 0; i < N_ROWS * N_COLS; ++i) {
    pressed[i] = 0;
  }

  // Enable GPIO interrupt on all row pins
  for (uint row = 0; row < N_ROWS; ++row) {
    gpio_set_irq_enabled(row_pin[row], GPIO_IRQ_EDGE_RISE, true);
  }

  // Note active_col is set here and used in the gpio callback to write
  // into the right location of `pressed`.
  // l61_printf("enabling irq on row %d\n", row);

  for (active_col = 0; active_col < N_COLS; ++active_col) {
    // l61_printf("Polling for row %d, col %d\n", row, active_col);
    // Enable the rising edge interrupt for our input pins
    // Send the high signal in the active column
    gpio_put(col_pin[active_col], true);

    // Any key which is pressed here will trigger a rising edge interrupt on
    // the associated pin, which will call l61_keymatrix_gpio_callback to update
    // the `pressed` table.

    // Disable falling edge irq since we're about to turn the column pin low
    gpio_put(col_pin[active_col], false);

    // Before moving on to the next column, wait for all row pins to be low.
    // Otherwise, we will miss rising edges on the next iteration.
    // uint loop_count = 0;
    while ((gpio_get_all() & row_pin_mask) != 0) {
      // loop_count++;
    }
    // if (loop_count > 0)
    //   l61_printf("row pins at 0 after %d iterations\n", loop_count);
  }
  for (uint row = 0; row < N_ROWS; ++row) {
    gpio_set_irq_enabled(row_pin[row], GPIO_IRQ_EDGE_RISE, false);
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

bool l61_keymatrix_is_key_pressed(uint index) {
  return pressed[index];
}

bool l61_keymatrix_is_fn_key_pressed() {
  return pressed[L61_FN_KEY];
}

//-----------------------------------------------------------------------------
// IRQ callbacks
//-----------------------------------------------------------------------------

void l61_keymatrix_gpio_callback(uint gpio, uint32_t event_mask) {
  // l61_printf("gpio callback for pin %d, mask %d, active_col=%d\n", gpio,
  //            event_mask, active_col);

  // No need to call gpio_acknowledge_irq, it is called automatically
  if (event_mask & GPIO_IRQ_EDGE_RISE) {
    // If we see a rising edge, then the key identified by the active row and
    // column is pressed.
    uint row = l61_keymatrix_get_row(gpio);
    uint row_offset = l61_keymatrix_get_row_offset(row);
    pressed[row_offset + active_col] = true;
  }
}
