#include "key_matrix.h"
#include <stdio.h>
#include "hardware/gpio.h"

//--------------------------
// Setup
//--------------------------

void setup_keymatrix() {
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

  printf("Key matrix pins initialized\n");
}
