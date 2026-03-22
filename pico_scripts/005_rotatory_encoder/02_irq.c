#include <stdio.h>
#include "pico/stdlib.h"

#define CLK_PIN 0
#define DT_PIN 1
#define SW_PIN 2

volatile int counter = 0;
volatile bool updated = false;
volatile bool button_pressed = false;

void gpio_callback(uint gpio, uint32_t events) {
    static int previousCLK = 1;

    if (gpio == CLK_PIN) {
        int currentCLK = gpio_get(CLK_PIN);

        if (currentCLK != previousCLK) {
            if (gpio_get(DT_PIN) != currentCLK) {
                counter++;
            } else {
                counter--;
            }

            updated = true;
            previousCLK = currentCLK;
        }
    }

    if (gpio == SW_PIN) {
        if (!(gpio_get(SW_PIN))) {
            button_pressed = true;  // just set flag
        }
    }
}

int main() {
    stdio_init_all();

    gpio_init(CLK_PIN);
    gpio_set_dir(CLK_PIN, GPIO_IN);
    gpio_pull_up(CLK_PIN);

    gpio_init(DT_PIN);
    gpio_set_dir(DT_PIN, GPIO_IN);
    gpio_pull_up(DT_PIN);

    gpio_init(SW_PIN);
    gpio_set_dir(SW_PIN, GPIO_IN);
    gpio_pull_up(SW_PIN);

    sleep_ms(2000);
    printf("====== IRQ Rotatory Encoder ======\n");

    gpio_set_irq_enabled_with_callback(
        CLK_PIN,
        GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL,
        true,
        &gpio_callback
    );

    gpio_set_irq_enabled(
        SW_PIN,
        GPIO_IRQ_EDGE_FALL,
        true
    );

    uint64_t last_button_time = 0;

    while (1) {
        if (updated) {
            printf("Position: %d\n", counter);
            updated = false;
        }

        // Debounced button handling in main loop
        if (button_pressed) {
            uint64_t now = time_us_64();

            if (now - last_button_time > 200000) { // 200ms debounce
                printf("Button pressed\n");
                last_button_time = now;
            }

            button_pressed = false;
        }

        tight_loop_contents();
    }
}