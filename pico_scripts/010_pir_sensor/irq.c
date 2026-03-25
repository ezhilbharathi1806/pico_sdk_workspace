#include <stdio.h>
#include "pico/stdlib.h"

#define PIR_PIN 15

void pir_callback(uint gpio, uint32_t events) {
    if (gpio_get(PIR_PIN)) {
        printf("Motion Detected!\n");
    } else {
        printf("Motion Ended\n");
    }
}

int main() {
    stdio_init_all();

    gpio_init(PIR_PIN);
    gpio_set_dir(PIR_PIN, GPIO_IN);

    gpio_set_irq_enabled_with_callback(
        PIR_PIN,
        GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL,
        true,
        &pir_callback
    );

    while (1) {
        tight_loop_contents();
    }
}