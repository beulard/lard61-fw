// Print stuff via serial/USB communication

#include <stdio.h>
#include "hardware/gpio.h"
#include "pico/time.h"

// On Pico
#define LED_PIN PICO_DEFAULT_LED_PIN
// On lard61 target
// #define LED_PIN 7

int main() {
  stdio_init_all();

  gpio_init(LED_PIN);
  gpio_set_dir(LED_PIN, GPIO_OUT);

  bool led_state = false;

  while (true) {
    printf("Hello from lard61!\n");
    led_state = !led_state;
    gpio_put(LED_PIN, led_state);
    sleep_ms(500);
  }

}
