#include <stdio.h>
#include <stdint.h>>
#include <pico/stdlib.h>
#include <hardware/gpio.h>

// Define the input pin (button)
#define BUTTON_PIN 14

// Interrupt handler function
void gpio_callback(uint gpio, uint32_t events) {
    if (events & GPIO_IRQ_EDGE_RISE) {
        printf("Button pressed on GPIO %d (Rising Edge)\n", gpio);
        sleep_ms(10);  //debounce delay
    }
    if (events & GPIO_IRQ_EDGE_FALL) {
        printf("Button released on GPIO %d (Falling Edge)\n", gpio);
        sleep_ms(10);  //debounce delay
    }
}

int main() {
    // Initialize stdio for printf output (via USB)
    stdio_init_all();

    // Initialize the GPIO pin
    gpio_init(BUTTON_PIN);
    gpio_set_dir(BUTTON_PIN, GPIO_IN);
    gpio_pull_up(BUTTON_PIN); // Use internal pull-up resistor

    // Enable interrupt on rising and falling edge
    gpio_set_irq_enabled_with_callback(
        BUTTON_PIN,
        GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL,
        true,
        &gpio_callback
    );

    // Main loop can do other tasks
    while (1) {
        tight_loop_contents(); // low power idle
    }
}
