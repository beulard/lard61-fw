/*
** file: lard61_keymatrix.h
** author: beulard (Matthias Dubouchet)
** creation date: 11/07/2024
*/

#ifndef _LARD61_KEYMATRIX_H
#define _LARD61_KEYMATRIX_H

#include "pico/types.h"

#define N_ROWS 5
#define N_COLS 14
#define TOTAL_KEYS 61

//-----------------------------------------------------------------------------
// Public API
//-----------------------------------------------------------------------------

// Setup GPIOs of the key matrix
void l61_keymatrix_setup();
// Get the row index corresponding to a GPIO pin
uint l61_keymatrix_get_row(uint gpio);
// Get the first key index for a given row index
// e.g. row 2 => first key is key 28
uint l61_keymatrix_get_row_offset(uint row);

// Query the state of all keys on the keyboard
void l61_keymatrix_update();
// Print out what keys are pressed according to the last call
// to l61_keymatrix_update
void l61_keymatrix_report();
// Returns true if switch at index is pressed down
bool l61_keymatrix_is_key_pressed(uint index);
// Returns true if the Fn/layer key is pressed
bool l61_keymatrix_is_fn_key_pressed();

#endif /* _LARD61_KEYMATRIX_H */
