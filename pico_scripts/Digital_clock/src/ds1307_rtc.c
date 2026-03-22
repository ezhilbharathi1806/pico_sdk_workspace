#include "ds1307_rtc.h"

#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"

// ------------ helper functions--------------
// Helper: BCD to Decimal conversion
uint8_t bcd_to_dec(uint8_t val) {
    //return ((val / 16) * 10) + (val % 16);
    return ((val >> 4) * 10) + (val & 0x0F);  // Faster than division
}

// Helper: Decimal to BCD conversion
uint8_t dec_to_bcd(uint8_t val) {
    //return ((val / 10) * 16) + (val % 10);
    return ((val / 10) << 4) | (val % 10);
}

// ------------ I2C helper functions--------------
void ds1307_write_register(i2c_inst_t *i2c, uint8_t reg, uint8_t value){
    uint8_t data[2] = {reg, value};
    i2c_write_blocking(i2c, DS1307_ADDR, data, 2, false);
}

void ds1307_read_register(i2c_inst_t *i2c, uint8_t reg, uint8_t *buf, uint8_t len){
    i2c_write_blocking(i2c, DS1307_ADDR, &reg, 1, true);
    i2c_read_blocking(i2c, DS1307_ADDR, buf, len, false);
}

// ------------ ds1307 initialization ------------
void ds1307_init(i2c_inst_t *i2c){
    uint8_t seconds;
    ds1307_read_register(i2c, DS1307_REG_SECONDS, &seconds, 1);      // Read seconds register

     // Check if oscillator is disabled (bit 7 = 1)
     if (seconds & 0x80){
        // Enable oscillator by clearing bit 7
        ds1307_write_register(i2c, DS1307_REG_SECONDS, seconds & 0x7f);
     }

     //Disable square wave output
     ds1307_write_register(i2c, DS1307_REG_CONTROL, 0x00);
}

// ------------ Set date and time ------------
void ds1307_set_datetime(i2c_inst_t *i2c, ds1307_datetime_t *dt){
    uint8_t buffer[8];
    buffer[0] = DS1307_REG_SECONDS;
    buffer[1] = dec_to_bcd(dt->seconds) & 0x7F;
    buffer[2] = dec_to_bcd(dt->minutes);
    buffer[3] = dec_to_bcd(dt->hours) & 0x3F;
    buffer[4] = dec_to_bcd(dt->day);
    buffer[5] = dec_to_bcd(dt->date);
    buffer[6] = dec_to_bcd(dt->month);
    buffer[7] = dec_to_bcd(dt->year);
    i2c_write_blocking(i2c, DS1307_ADDR, buffer, 8, false);
}

// ------------ read current date and time ------------
void ds1307_get_datetime(i2c_inst_t *i2c, ds1307_datetime_t *dt){
    uint8_t data[7];
    ds1307_read_register(i2c, DS1307_REG_SECONDS, data, 7);
    
    dt->seconds = bcd_to_dec(data[0] & 0x7F);
    dt->minutes = bcd_to_dec(data[1] & 0x7F);
    dt->hours   = bcd_to_dec(data[2] & 0x3F);
    dt->day     = bcd_to_dec(data[3] & 0x07);
    dt->date    = bcd_to_dec(data[4] & 0x3F);
    dt->month   = bcd_to_dec(data[5] & 0x1F);
    dt->year    = bcd_to_dec(data[6]);
}

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
