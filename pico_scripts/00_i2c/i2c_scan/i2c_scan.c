#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"

#define I2C_PORT i2c0
#define SDA_PIN 4
#define SCL_PIN 5
#define BAUD_RATE 100000

int main() {
    stdio_init_all(); // Initialize standard IO (UART/USB CDC)

    // Initialize I2C at defined baud rate and pins
    i2c_init(I2C_PORT, BAUD_RATE);
    gpio_set_function(SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(SDA_PIN);
    gpio_pull_up(SCL_PIN);

    printf("I2C device scan started\n");

    while (1) {
        for(uint8_t addr = 0x08; addr <= 0x77; addr++){
          int result;
          uint8_t rxdata = 0;
          result = i2c_read_blocking(I2C_PORT, addr, &rxdata, 1, false);
          printf(result<0 ? ".":"\n i2c device found at address 0x%02X\n",addr);
          sleep_ms(5);    // Small delay between probe
        }
        sleep_ms(100); 
    }
    return 0;
}
