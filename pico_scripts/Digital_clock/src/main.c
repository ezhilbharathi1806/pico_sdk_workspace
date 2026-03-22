#include "ssd1306.h"
#include "ds1307_rtc.h"

#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"

// I2C configuration
#define I2C_PORT    i2c0
#define I2C0_SDA    4
#define I2C0_SCL    5
#define BAUD_RATE 100000

int main(void){
    stdio_init_all();

    //Initialize I2C
    i2c_init(I2C_PORT, BAUD_RATE);
    gpio_set_function(I2C0_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C0_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C0_SDA);
    gpio_pull_up(I2C0_SCL);

    //Initialize SSD1306 Oled
    ssd1306_t display;
    display.external_vcc = false;
    ssd1306_init(&display, 128, 64, 0x3c, I2C_PORT);
    ssd1306_clear(&display);

    //Initilize DS1307 RTC
    ds1307_init(I2C_PORT);

    // Set initial time using struct
    // Uncomment to set time (only needed once)
    ds1307_datetime_t initial_time = {
        .seconds = 0,
        .minutes = 36,
        .hours = 18,      // 5:08 PM
        .day = 6,         // Saturday
        .date = 25,
        .month = 10,
        .year = 25        // 2025
    };
    ds1307_set_datetime(I2C_PORT, &initial_time);

    ds1307_datetime_t current_time;
    char time_str[20];
    char date_str[20];

    time_12h_t current_time_12h;
    char time_12h_str[20];

    while(1){
        // Read time from DS130
        ds1307_get_datetime(I2C_PORT, &current_time);

        // Read time from DS1307
        snprintf(time_str, sizeof(time_str), "%02d:%02d:%02d",
                current_time.hours, current_time.minutes, current_time.seconds);

        // Format date string (DD/MM/YYYY)
        snprintf(date_str, sizeof(date_str), "%02d/%02d/20%02d",
                current_time.date, current_time.month, current_time.year);

        convert_to_12h(&current_time, &current_time_12h);
        snprintf(time_12h_str, sizeof(time_12h_str), "%02d:%02d:%02d %s",
                current_time_12h.hours, current_time_12h.minutes, current_time_12h.seconds,
                current_time_12h.is_pm ? "PM" : "AM");

        // Display on Oled
        ssd1306_clear(&display);

        // Draw title
        ssd1306_draw_string(&display, 0, 0, 1, "DIGITAL CLOCK");
        
        // Draw time (larger font)
        ssd1306_draw_string(&display, 0, 16, 1, time_str);
        
        // Draw date
        ssd1306_draw_string(&display, 0, 32, 1, date_str);

        ssd1306_draw_string(&display, 0, 48, 1, time_12h_str);
        
        ssd1306_show(&display);
        

        sleep_ms(500);
    }
    return 0;
}
