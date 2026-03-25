/*
| PIR Pin | Pico               |
| ------- | ------------------ |
| VCC     | 5V (VBUS) or 3.3V* |
| GND     | GND                |
| OUT     | GPIO 15            |
*/

#include <stdio.h>
#include "pico/stdlib.h"

#define PIR_PIN 15

int main() {
    stdio_init_all();

    gpio_init(PIR_PIN);
    gpio_set_dir(PIR_PIN, GPIO_IN);
    gpio_pull_down(PIR_PIN);

    printf("PIR Motion Sensor Example\n");

    int last_state = 0;

    while (1) {
        int current_state = gpio_get(PIR_PIN);

        // Detect change (avoid spam printing)
        if (current_state != last_state) {
            if (current_state) {
                printf("Motion Detected!\n");
            } else {
                printf("Motion Ended\n");
            }
            last_state = current_state;
        }

        sleep_ms(10);  // small delay
    }
}