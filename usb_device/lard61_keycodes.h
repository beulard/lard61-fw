/*
** file: lard61_keycodes.h
** author: beulard (Matthias Dubouchet)
** creation date: 12/07/2024
**
** Convert from lard61 keymatrix index to HID keycodes.
*/

#ifndef _LARD61_KEYCODES_H
#define _LARD61_KEYCODES_H

#include "class/hid/hid.h"
#include "lard61_keymatrix.h"

// HID keycode associated to each key
const uint8_t l61_hid_keycode[N_ROWS * N_COLS] = {
    // Row 0: index 0-13
    HID_KEY_ESCAPE,
    HID_KEY_1,
    HID_KEY_2,
    HID_KEY_3,
    HID_KEY_4,
    HID_KEY_5,
    HID_KEY_6,
    HID_KEY_7,
    HID_KEY_8,
    HID_KEY_9,
    HID_KEY_0,
    HID_KEY_MINUS,
    HID_KEY_EQUAL,
    HID_KEY_BACKSPACE,
    // Row 1: index 14-27
    HID_KEY_TAB,
    HID_KEY_Q,
    HID_KEY_W,
    HID_KEY_E,
    HID_KEY_R,
    HID_KEY_T,
    HID_KEY_Y,
    HID_KEY_U,
    HID_KEY_I,
    HID_KEY_O,
    HID_KEY_P,
    HID_KEY_BRACKET_LEFT,
    HID_KEY_BRACKET_RIGHT,
    HID_KEY_BACKSLASH,
    // Row 2: index 28-40
    HID_KEY_CAPS_LOCK,
    HID_KEY_A,
    HID_KEY_S,
    HID_KEY_D,
    HID_KEY_F,
    HID_KEY_G,
    HID_KEY_H,
    HID_KEY_J,
    HID_KEY_K,
    HID_KEY_L,
    HID_KEY_SEMICOLON,
    HID_KEY_APOSTROPHE,
    HID_KEY_ENTER,
    // Row 3: index 41-52
    HID_KEY_SHIFT_LEFT,
    HID_KEY_Z,
    HID_KEY_X,
    HID_KEY_C,
    HID_KEY_V,
    HID_KEY_B,
    HID_KEY_N,
    HID_KEY_M,
    HID_KEY_COMMA,
    HID_KEY_PERIOD,
    HID_KEY_SLASH,
    HID_KEY_SHIFT_RIGHT,
    // Row 4: index 53-69
    HID_KEY_CONTROL_LEFT,
    HID_KEY_GUI_LEFT,
    HID_KEY_ALT_LEFT,
    HID_KEY_SPACE,
    // Space bar is on column 3, but Right alt is on column 8
    // So we need some NONE entries to pad
    HID_KEY_NONE,
    HID_KEY_NONE,
    HID_KEY_NONE,
    HID_KEY_NONE,
    HID_KEY_ALT_RIGHT,
    HID_KEY_GUI_RIGHT,
    HID_KEY_NONE, // Function/layer key
    HID_KEY_CONTROL_RIGHT,
    // The rest of the keymatrix does not correspond to a key
    HID_KEY_NONE,
    HID_KEY_NONE,
    HID_KEY_NONE,
    HID_KEY_NONE,
    HID_KEY_NONE,
};

// Keycode associated with each key, when the function key is also pressed
// Differences with the table above:
// - Backtick (GRAVE) on the top left ESC key
// - F1-F12 keys on the top layer
// - Directional arrows on WASD, PG_UP on Q, PG_DOWN on E
// - Directional arrows on PL:" for one-handed motions
// - Delete key on backspace
// - HOME on R, END on F
const uint8_t l61_hid_keycode_fn[N_ROWS * N_COLS] = {
    // Row 0: index 0-13
    HID_KEY_GRAVE,
    HID_KEY_F1,
    HID_KEY_F2,
    HID_KEY_F3,
    HID_KEY_F4,
    HID_KEY_F5,
    HID_KEY_F6,
    HID_KEY_F7,
    HID_KEY_F8,
    HID_KEY_F9,
    HID_KEY_F10,
    HID_KEY_F11,
    HID_KEY_F12,
    HID_KEY_DELETE,
    // Row 1: index 14-27
    HID_KEY_TAB,
    HID_KEY_PAGE_UP,
    HID_KEY_ARROW_UP,
    HID_KEY_PAGE_DOWN,
    HID_KEY_HOME,
    HID_KEY_T,
    HID_KEY_Y,
    HID_KEY_U,
    HID_KEY_I,
    HID_KEY_O,
    HID_KEY_ARROW_UP,
    HID_KEY_BRACKET_LEFT,
    HID_KEY_BRACKET_RIGHT,
    HID_KEY_BACKSLASH,
    // Row 2: index 28-40
    HID_KEY_CAPS_LOCK,
    HID_KEY_ARROW_LEFT,
    HID_KEY_ARROW_DOWN,
    HID_KEY_ARROW_RIGHT,
    HID_KEY_END,
    HID_KEY_G,
    HID_KEY_H,
    HID_KEY_J,
    HID_KEY_K,
    HID_KEY_ARROW_LEFT,
    HID_KEY_ARROW_DOWN,
    HID_KEY_ARROW_RIGHT,
    HID_KEY_ENTER,
    // Row 3: index 41-52
    HID_KEY_SHIFT_LEFT,
    HID_KEY_Z,
    HID_KEY_X,
    HID_KEY_C,
    HID_KEY_V,
    HID_KEY_B,
    HID_KEY_N,
    HID_KEY_M,
    HID_KEY_COMMA,
    HID_KEY_PERIOD,
    HID_KEY_SLASH,
    HID_KEY_SHIFT_RIGHT,
    // Row 4: index 53-69
    HID_KEY_CONTROL_LEFT,
    HID_KEY_GUI_LEFT,
    HID_KEY_ALT_LEFT,
    HID_KEY_SPACE,
    // Space bar is on column 3, but Right alt is on column 8
    // So we need some NONE entries to pad
    HID_KEY_NONE,
    HID_KEY_NONE,
    HID_KEY_NONE,
    HID_KEY_NONE,
    HID_KEY_ALT_RIGHT,
    HID_KEY_GUI_RIGHT,
    HID_KEY_NONE, // Function/layer key
    HID_KEY_CONTROL_RIGHT,
    // The rest of the keymatrix does not correspond to a key
    HID_KEY_NONE,
    HID_KEY_NONE,
    HID_KEY_NONE,
    HID_KEY_NONE,
    HID_KEY_NONE,
};

#endif /* _LARD61_KEYCODES_H */
