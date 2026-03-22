#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "ssd1306.h"

// I2C configuration
#define I2C_PORT i2c0
#define I2C_SDA_PIN 4
#define I2C_SCL_PIN 5

int main() {
    stdio_init_all();
    
    // Initialize I2C at 400kHz
    i2c_init(I2C_PORT, 400000);
    gpio_set_function(I2C_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA_PIN);
    gpio_pull_up(I2C_SCL_PIN);
    
    // Small delay for display to power up
    sleep_ms(250);
    
    printf("Initializing SSD1306...\n");
    
    // Initialize SSD1306 (address 0x3C for 128x64 display)
    ssd1306_t disp;
    disp.external_vcc = false;  // Use internal charge pump
    ssd1306_init(&disp, 128, 64, 0x3C, I2C_PORT);
    
    // Clear the display
    ssd1306_clear(&disp);
    
    // Draw "Hello World!" text at position (0, 0)
    ssd1306_draw_string(&disp, 0, 0, 1, "Hello World!");
    
    // Draw additional text
    ssd1306_draw_string(&disp, 0, 16, 1, "Pico + SSD1306");
    ssd1306_draw_string(&disp, 0, 32, 1, "C SDK Example");
    
    // Show the buffer on display
    ssd1306_show(&disp);
    
    printf("Display initialized!\n");
    
    // Blink pattern on display
    while (1) {
        // Invert display
        ssd1306_draw_string(&disp, 0, 48, 1, "Blinking...");
        ssd1306_show(&disp);
        sleep_ms(1000);
        
        // Clear only the blinking area (x=0, y=48, width=128, height=16)
        ssd1306_clear_square(&disp, 0, 48, 128, 16);
        ssd1306_show(&disp);
        sleep_ms(1000);
    }
    
    return 0;
}
