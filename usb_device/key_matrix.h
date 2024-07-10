#ifndef _KEY_MATRIX_H
#define _KEY_MATRIX_H 1

#include "pico/stdlib.h"

#define N_ROWS 5
#define N_COLS 14
#define TOTAL_KEYS 61

static const uint n_keys_in_row[N_ROWS] = {14, 14, 13, 12, 8};

static const uint row_pins[N_ROWS] = {
    19,  // row0
    20,  // row1
    21,  // row2
    22,  // row3
    18,  // row4
};

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

// Setup GPIOs of the key matrix
void keymatrix_setup();
// Get the row index corresponding to a GPIO pin
uint keymatrix_get_row(uint gpio);
// Get the first key index for a given row index
// e.g. row 2 => first key is key 28
uint keymatrix_get_row_offset(uint row);

#endif /* _KEY_MATRIX_H */
