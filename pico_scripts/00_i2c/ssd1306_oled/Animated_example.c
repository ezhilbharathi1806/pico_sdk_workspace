#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "ssd1306.h"

#define I2C_PORT i2c0
#define I2C_SDA_PIN 4
#define I2C_SCL_PIN 5

int main() {
    stdio_init_all();
    
    // Initialize I2C
    i2c_init(I2C_PORT, 400000);
    gpio_set_function(I2C_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA_PIN);
    gpio_pull_up(I2C_SCL_PIN);
    
    sleep_ms(250);
    
    // Initialize display
    ssd1306_t disp;
    disp.external_vcc = false;
    ssd1306_init(&disp, 128, 64, 0x3C, I2C_PORT);
    
    int x = 0;
    int direction = 1;
    
    while (1) {
        // Clear display
        ssd1306_clear(&disp);
        
        // Draw moving text
        ssd1306_draw_string(&disp, x, 28, 1, "Hello Pico!");
        
        // Draw border
        ssd1306_draw_empty_square(&disp, 0, 0, 128, 64);
        
        // Update display
        ssd1306_show(&disp);
        
        // Move text
        x += direction * 2;
        if (x > 40 || x < 0) {
            direction *= -1;
        }
        
        sleep_ms(50);
    }
    
    return 0;
}

