#ifndef _BOARDS_LARD61_H
#define _BOARDS_LARD61_H

// For board detection
#define LARD61

// --- LED ---
#ifndef PICO_DEFAULT_LED_PIN
#define PICO_DEFAULT_LED_PIN 7
#endif
// no PICO_DEFAULT_WS2812_PIN


// --- FLASH ---
#define PICO_BOOT_STAGE2_CHOOSE_W25Q080 1

#ifndef PICO_FLASH_SPI_CLKDIV
#define PICO_FLASH_SPI_CLKDIV 2
#endif

#ifndef PICO_FLASH_SIZE_BYTES
#define PICO_FLASH_SIZE_BYTES (4 * 1024 * 1024)
#endif

// TODO(mdu) check with rp2040_chip_version
#ifndef PICO_RP2040_B0_SUPPORTED
#define PICO_RP2040_B0_SUPPORTED 1
#endif

#endif
