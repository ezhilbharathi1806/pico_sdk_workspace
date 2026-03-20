#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"

// I2C configuration
#define I2C_PORT    i2c0
#define I2C0_SDA_PIN    4
#define I2C0_SCL_PIN    5
#define BAUD_RATE 400000
#define DS1307_ADDR 0x68

// DS1307 Register Addresses
#define DS1307_REG_SECONDS   0x00
#define DS1307_REG_MINUTES   0x01
#define DS1307_REG_HOURS     0x02
#define DS1307_REG_DAY       0x03
#define DS1307_REG_DATE      0x04
#define DS1307_REG_MONTH     0x05
#define DS1307_REG_YEAR      0x06
#define DS1307_REG_CONTROL   0x07

// Simple datetime structure (always 24-hour internally)
typedef struct {
    uint8_t seconds;    // 0-59
    uint8_t minutes;    // 0-59
    uint8_t hours;      // 0-23 (always 24-hour format)
    uint8_t day;        // 1-7 (1=Sunday)
    uint8_t date;       // 1-31
    uint8_t month;      // 1-12
    uint8_t year;       // 0-99 (2000-2099)
} ds1307_datetime_t;

// Structure for 12-hour format display
typedef struct {
    uint8_t hours;      // 1-12
    uint8_t minutes;    // 0-59
    uint8_t seconds;    // 0-59
    bool is_pm;         // false = AM, true = PM
} time_12h_t;

// ------------ Helper Functions --------------

// BCD to Decimal
uint8_t bcd_to_dec(uint8_t val) {
    return ((val >> 4) * 10) + (val & 0x0F);
}

// Decimal to BCD
uint8_t dec_to_bcd(uint8_t val) {
    return ((val / 10) << 4) | (val % 10);
}

// ------------ 24H to 12H Conversion Function --------------
void convert_to_12h(const ds1307_datetime_t *dt, time_12h_t *time_12h) {
    time_12h->minutes = dt->minutes;
    time_12h->seconds = dt->seconds;
    
    if (dt->hours == 0) {
        // Midnight: 00:00 → 12:00 AM
        time_12h->hours = 12;
        time_12h->is_pm = false;
    } else if (dt->hours < 12) {
        // Morning: 1-11 AM
        time_12h->hours = dt->hours;
        time_12h->is_pm = false;
    } else if (dt->hours == 12) {
        // Noon: 12:00 PM
        time_12h->hours = 12;
        time_12h->is_pm = true;
    } else {
        // Afternoon/Evening: 13-23 → 1-11 PM
        time_12h->hours = dt->hours - 12;
        time_12h->is_pm = true;
    }
}

// ------------ I2C Helper Functions --------------

void ds1307_write_register(uint8_t reg, uint8_t value) {
    uint8_t data[2] = {reg, value};
    i2c_write_blocking(I2C_PORT, DS1307_ADDR, data, 2, false);
}

void ds1307_read_register(uint8_t reg, uint8_t *buf, uint8_t len) {
    i2c_write_blocking(I2C_PORT, DS1307_ADDR, &reg, 1, true);
    i2c_read_blocking(I2C_PORT, DS1307_ADDR, buf, len, false);
}

// ------------ DS1307 Initialization ------------

void ds1307_init(void) {
    uint8_t seconds;
    ds1307_read_register(DS1307_REG_SECONDS, &seconds, 1);

    // Enable oscillator if disabled (clear bit 7)
    if (seconds & 0x80) {
        ds1307_write_register(DS1307_REG_SECONDS, seconds & 0x7F);
    }

    // Disable square wave output
    ds1307_write_register(DS1307_REG_CONTROL, 0x00);
}

// ------------ Set Date and Time (Always 24H) ------------

void ds1307_set_datetime(const ds1307_datetime_t *dt) {
    // Always set in 24-hour format (bit 6 = 0)
    ds1307_write_register(DS1307_REG_SECONDS, dec_to_bcd(dt->seconds) & 0x7F);
    ds1307_write_register(DS1307_REG_MINUTES, dec_to_bcd(dt->minutes));
    ds1307_write_register(DS1307_REG_HOURS, dec_to_bcd(dt->hours) & 0x3F);  // 24h mode
    ds1307_write_register(DS1307_REG_DAY, dec_to_bcd(dt->day));
    ds1307_write_register(DS1307_REG_DATE, dec_to_bcd(dt->date));
    ds1307_write_register(DS1307_REG_MONTH, dec_to_bcd(dt->month));
    ds1307_write_register(DS1307_REG_YEAR, dec_to_bcd(dt->year));
}

// ------------ Read Current Date and Time (Always 24H) ------------

void ds1307_get_datetime(ds1307_datetime_t *dt) {
    uint8_t data[7];
    ds1307_read_register(DS1307_REG_SECONDS, data, 7);
    
    dt->seconds = bcd_to_dec(data[0] & 0x7F);
    dt->minutes = bcd_to_dec(data[1] & 0x7F);
    dt->hours   = bcd_to_dec(data[2] & 0x3F);  // Always read as 24h
    dt->day     = bcd_to_dec(data[3] & 0x07);
    dt->date    = bcd_to_dec(data[4] & 0x3F);
    dt->month   = bcd_to_dec(data[5] & 0x1F);
    dt->year    = bcd_to_dec(data[6]);
}

// ------------ Print Functions ------------

// Print in 24-hour format (default)
void print_datetime_24h(const ds1307_datetime_t *dt) {
    const char* days[] = {"", "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
    
    printf("%s, 20%02d-%02d-%02d  %02d:%02d:%02d (24h)\n",
           days[dt->day], 
           dt->year, dt->month, dt->date,
           dt->hours, dt->minutes, dt->seconds);
}

// Print in 12-hour format (with conversion)
void print_datetime_12h(const ds1307_datetime_t *dt) {
    const char* days[] = {"", "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
    
    // Convert to 12-hour format
    time_12h_t time_12h;
    convert_to_12h(dt, &time_12h);
    
    printf("%s, 20%02d-%02d-%02d  %02d:%02d:%02d %s (12h)\n",
           days[dt->day], 
           dt->year, dt->month, dt->date,
           time_12h.hours, time_12h.minutes, time_12h.seconds,
           time_12h.is_pm ? "PM" : "AM");
}

// Print both formats side-by-side
void print_datetime_both(const ds1307_datetime_t *dt) {
    const char* days[] = {"", "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
    
    // Convert to 12h
    time_12h_t time_12h;
    convert_to_12h(dt, &time_12h);
    
    printf(" Date: %s, 20%02d-%02d-%02d \n", days[dt->day], dt->year, dt->month, dt->date);
    printf(" 24-hour: %02d:%02d:%02d    \n",dt->hours, dt->minutes, dt->seconds);
    printf(" 12-hour: %02d:%02d:%02d %s \n",time_12h.hours, time_12h.minutes, time_12h.seconds,time_12h.is_pm ? "PM" : "AM");
}

// ------------ Additional Utility Functions ------------

// Check if current time is AM or PM
bool is_pm(const ds1307_datetime_t *dt) {
    return dt->hours >= 12;
}

// Get hour in 12-hour format (returns just the hour)
uint8_t get_hour_12h(const ds1307_datetime_t *dt) {
    time_12h_t time_12h;
    convert_to_12h(dt, &time_12h);
    return time_12h.hours;
}

// ------------ Main ------------
int main() {
    stdio_init_all();

    // Initialize I2C
    i2c_init(I2C_PORT, BAUD_RATE);
    gpio_set_function(I2C0_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(I2C0_SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(I2C0_SDA_PIN);
    gpio_pull_up(I2C0_SCL_PIN);

    sleep_ms(2000);
    
    ds1307_init();

    // Set initial time (always in 24-hour format)
    ds1307_datetime_t initial_time = {
        .seconds = 0,
        .minutes = 30,
        .hours = 17,      // 5:30 PM (always use 24h: 0-23)
        .day = 6,         // Saturday
        .date = 15,
        .month = 11,
        .year = 25        // 2025
    };
    
    ds1307_set_datetime(&initial_time);
    
    printf("Reading time every second...\n");
    printf("(Displaying both 24h and 12h formats)\n\n");
    
    ds1307_datetime_t current_time;
    
    while (1) {
        // Read current time (always 24h)
        ds1307_get_datetime(&current_time);
        
        // Display in both formats
        print_datetime_both(&current_time);

        sleep_ms(1000);
    }

    return 0;
}
