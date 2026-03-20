#include <stdio.h>
#include<stdint.h>
#include<stdbool.h>
#include "pico/stdlib.h"

int main() {
    const uint8_t led_pin = 25;
    // Initialize LED pin
    gpio_init(led_pin);
    gpio_set_dir(led_pin, GPIO_OUT);

    // Initialize chosen serial port
    stdio_init_all();

    // Loop forever
    while (1) {

        // Blink LED
        printf("Blinking!\r\n");

        //led on
        gpio_put(led_pin, true);
        sleep_ms(500);

        //led off
        gpio_put(led_pin, false);
        sleep_ms(500);
    }
}