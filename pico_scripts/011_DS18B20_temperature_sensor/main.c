/*
DS18B20        Pico
---------      ----------------
VCC (Red)   →  3.3V (Pin 36)
GND (Black) →  GND (Pin 38)
DATA        →  GP2 (Pin 4)

4.7kΩ resistor:(4.7kΩ resistor between DATA ↔ VCC)
DATA --------/\/\/\-------- 3.3V*/
#include <stdio.h>
#include "pico/stdlib.h"

#define DS18B20_PIN 2

void onewire_write_bit(bool bit) {
    gpio_set_dir(DS18B20_PIN, GPIO_OUT);
    gpio_put(DS18B20_PIN, 0);
    sleep_us(bit ? 6 : 60);
    gpio_put(DS18B20_PIN, 1);
    sleep_us(bit ? 64 : 10);
}

bool onewire_read_bit() {
    gpio_set_dir(DS18B20_PIN, GPIO_OUT);
    gpio_put(DS18B20_PIN, 0);
    sleep_us(6);

    gpio_set_dir(DS18B20_PIN, GPIO_IN);
    sleep_us(9);
    bool bit = gpio_get(DS18B20_PIN);
    sleep_us(55);

    return bit;
}

void onewire_write_byte(uint8_t byte) {
    for (int i = 0; i < 8; i++) {
        onewire_write_bit(byte & 0x01);
        byte >>= 1;
    }
}

uint8_t onewire_read_byte() {
    uint8_t value = 0;
    for (int i = 0; i < 8; i++) {
        value >>= 1;
        if (onewire_read_bit()) {
            value |= 0x80;
        }
    }
    return value;
}

bool onewire_reset() {
    gpio_set_dir(DS18B20_PIN, GPIO_OUT);
    gpio_put(DS18B20_PIN, 0);
    sleep_us(480);

    gpio_set_dir(DS18B20_PIN, GPIO_IN);
    sleep_us(70);
    bool presence = !gpio_get(DS18B20_PIN);
    sleep_us(410);

    return presence;
}

float read_temperature() {
    if (!onewire_reset()) return -1000;

    onewire_write_byte(0xCC); // Skip ROM
    onewire_write_byte(0x44); // Convert T

    sleep_ms(750); // conversion time

    onewire_reset();
    onewire_write_byte(0xCC);
    onewire_write_byte(0xBE); // Read scratchpad

    uint8_t temp_l = onewire_read_byte();
    uint8_t temp_h = onewire_read_byte();

    int16_t raw = (temp_h << 8) | temp_l;
    return raw / 16.0f;
}

int main() {
    stdio_init_all();

    gpio_init(DS18B20_PIN);
    gpio_pull_up(DS18B20_PIN);

    while (true) {
        float temp = read_temperature();
        printf("Temperature: %.2f °C\n", temp);
        sleep_ms(500);
    }
}