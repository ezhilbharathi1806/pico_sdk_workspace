/*
 * This is a simple example to demonstrate how to use the SSD1306 OLED display with SPI interface on the Raspberry Pi Pico.
 * The code initializes the display and shows "Hello" on the screen.
 *
 * Connections:
| OLED Pin  | Pico GPIO | Notes        |
| --------- | --------- | ------------ |
| VCC       | 3.3V      |    Not 5V    |
| GND       | GND       |              |
| SCK (D0)  | GPIO 18   | SPI0 SCK     |
| MOSI (D1) | GPIO 19   | SPI0 TX      |
| CS        | GPIO 17   | Chip Select  |
| DC        | GPIO 16   | Data/Command |
| RES       | GPIO 20   | Reset        |
*/

#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/spi.h"

// SPI Defines
#define SPI_PORT spi0
#define PIN_MISO -1   // Not used
#define PIN_CS   17
#define PIN_SCK  18
#define PIN_MOSI 19
#define PIN_DC   16
#define PIN_RST  20

#define WIDTH 128
#define HEIGHT 64

uint8_t buffer[WIDTH * HEIGHT / 8];

// Basic 5x7 font for "Hello"
const uint8_t font[][5] = {
    {0x7F,0x08,0x08,0x08,0x7F}, // H
    {0x38,0x54,0x54,0x54,0x18}, // e
    {0x00,0x41,0x7F,0x40,0x00}, // l
    {0x00,0x41,0x7F,0x40,0x00}, // l
    {0x38,0x44,0x44,0x44,0x38}, // o
};

// Send command
void ssd1306_cmd(uint8_t cmd) {
    gpio_put(PIN_DC, 0);
    gpio_put(PIN_CS, 0);
    spi_write_blocking(SPI_PORT, &cmd, 1);
    gpio_put(PIN_CS, 1);
}

// Send data
void ssd1306_data(uint8_t *data, size_t len) {
    gpio_put(PIN_DC, 1);
    gpio_put(PIN_CS, 0);
    spi_write_blocking(SPI_PORT, data, len);
    gpio_put(PIN_CS, 1);
}

// Init display
void ssd1306_init() {
    gpio_put(PIN_RST, 0);
    sleep_ms(50);
    gpio_put(PIN_RST, 1);

    ssd1306_cmd(0xAE); // Display OFF
    ssd1306_cmd(0xA8); ssd1306_cmd(0x3F);
    ssd1306_cmd(0xD3); ssd1306_cmd(0x00);
    ssd1306_cmd(0x40);
    ssd1306_cmd(0xA1);
    ssd1306_cmd(0xC8);
    ssd1306_cmd(0xDA); ssd1306_cmd(0x12);
    ssd1306_cmd(0x81); ssd1306_cmd(0x7F);
    ssd1306_cmd(0xA4);
    ssd1306_cmd(0xA6);
    ssd1306_cmd(0xD5); ssd1306_cmd(0x80);
    ssd1306_cmd(0x8D); ssd1306_cmd(0x14);
    ssd1306_cmd(0xAF); // Display ON
}

// Clear buffer
void clear_buffer() {
    for (int i = 0; i < sizeof(buffer); i++) buffer[i] = 0;
}

// Draw char (very basic)
void draw_char(int x, int page, const uint8_t *ch) {
    for (int i = 0; i < 5; i++) {
        buffer[page * WIDTH + x + i] = ch[i];
    }
}

// Update display
void ssd1306_update() {
    for (int page = 0; page < 8; page++) {
        ssd1306_cmd(0xB0 + page);
        ssd1306_cmd(0x00);
        ssd1306_cmd(0x10);
        ssd1306_data(&buffer[WIDTH * page], WIDTH);
    }
}

int main() {
    stdio_init_all();

    // SPI init
    spi_init(SPI_PORT, 1000 * 1000); // 1 MHz
    gpio_set_function(PIN_SCK, GPIO_FUNC_SPI);
    gpio_set_function(PIN_MOSI, GPIO_FUNC_SPI);

    // Control pins
    gpio_init(PIN_CS);
    gpio_set_dir(PIN_CS, GPIO_OUT);
    gpio_put(PIN_CS, 1);

    gpio_init(PIN_DC);
    gpio_set_dir(PIN_DC, GPIO_OUT);

    gpio_init(PIN_RST);
    gpio_set_dir(PIN_RST, GPIO_OUT);

    ssd1306_init();

    clear_buffer();

    // Draw "Hello"
    draw_char(0, 0, font[0]);
    draw_char(6, 0, font[1]);
    draw_char(12, 0, font[2]);
    draw_char(18, 0, font[3]);
    draw_char(24, 0, font[4]);

    ssd1306_update();

    while (1) {
        tight_loop_contents();
    }
}