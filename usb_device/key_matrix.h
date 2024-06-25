#ifndef _KEY_MATRIX_H
#define _KEY_MATRIX_H 1

#include "pico/stdlib.h"

#define N_ROWS 5
#define N_COLS 14

static const uint row_pins[N_ROWS] = {
    19,  // row0
    20,  // row1
    21,  // row2
    22,  // row3
    18,  // row4
};

static const uint col_pins[N_COLS] = {
    23,  // col0
    25,  // col1
    26,  // col2
    24,  // col3
    27,  // col4
    28,  // col5
    29,  // col6
    // Also used for uart comm on dev Pico
    29,  // 0,   // col7
    29,  // 1,   // col8
    2,   // col9
    3,   // col10
    4,   // col11
    5,   // col12
    6,   // col12
};

// Setup GPIOs of the key matrix
void setup_keymatrix();

#endif /* _KEY_MATRIX_H */