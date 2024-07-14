#ifndef _BOARDS_LARD61_H
#define _BOARDS_LARD61_H

// For board detection
#define LARD61

// --- LED ---
#define PICO_DEFAULT_LED_PIN 7

// --- XOSC ---
// For some reason, oscillator is slightly slow to start on the lard61.
// This causes the firmware not to boot when plugging in the USB power.
// Adding this delay multiplier fixes the issue.
// Empirically, 4 is enough to eliminate the problem in 90% of boots,
// so 8 should be safe.
#define PICO_XOSC_STARTUP_DELAY_MULTIPLIER 8

// --- FLASH ---
#define PICO_BOOT_STAGE2_CHOOSE_W25Q080 1
#define PICO_FLASH_SPI_CLKDIV 2
#define PICO_FLASH_SIZE_BYTES (4 * 1024 * 1024)

// lard61 has RP2040 chip version 2
#define PICO_RP2040_B0_SUPPORTED 0

#endif /* _BOARDS_LARD61_H */
