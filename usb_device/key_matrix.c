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

uint get_row(uint gpio) {
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

uint get_row_offset(uint row) {
  for (uint i = 0; i < row; ++i) {
  }
}