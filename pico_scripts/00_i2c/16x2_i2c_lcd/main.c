#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "lcd_i2c.h"

#define I2C_PORT i2c0
#define SDA_PIN 4
#define SCL_PIN 5

int main() {
    stdio_init_all();
    
    // Initialize I2C at 100 kHz
    i2c_init(I2C_PORT, 100 * 1000);
    gpio_set_function(SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(SDA_PIN);
    gpio_pull_up(SCL_PIN);
    
    sleep_ms(2000);  // Wait for serial
    
    printf("LCD I2C Example\n");
    
    // Initialize LCD
    lcd_init(I2C_PORT, LCD_ADDR);
    
    // Display static text
    lcd_set_cursor(0, 0);
    lcd_print("Hello, Pico!");
    lcd_set_cursor(1, 0);
    lcd_print("LCD I2C Driver");
    
    sleep_ms(3000);
    
    // Counter example
    int count = 0;
    while (1) {
        lcd_clear();
        lcd_set_cursor(0, 0);
        lcd_print("Counter:");
        
        lcd_set_cursor(1, 0);
        char buffer[17];
        snprintf(buffer, sizeof(buffer), "Count: %d", count);
        lcd_print(buffer);
        
        count++;
        sleep_ms(1000);
    }
    
    return 0;
}
