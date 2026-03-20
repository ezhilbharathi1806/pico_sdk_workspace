
#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"

// I2C configuration
#define I2C_PORT    i2c0
#define I2C0_SDA_PIN    4
#define I2C0_SCL_PIN    5
#define BAUD_RATE 400000
#define DS1307_ADDR 0x68        // default I2C address

// DS1307 Register Addresses
#define DS1307_REG_SECONDS   0x00
#define DS1307_REG_MINUTES   0x01
#define DS1307_REG_HOURS     0x02
#define DS1307_REG_DAY       0x03
#define DS1307_REG_DATE      0x04
#define DS1307_REG_MONTH     0x05
#define DS1307_REG_YEAR      0x06
#define DS1307_REG_CONTROL   0x07

//date and time structure
typedef struct {
    uint8_t seconds;    // 0-59
    uint8_t minutes;    // 0-59
    uint8_t hours;      // 0-23 (24-hour format)
    uint8_t day;        // 1-7 (1=Sunday)
    uint8_t date;       // 1-31
    uint8_t month;      // 1-12
    uint8_t year;       // 0-99 (2000-2099)
} ds1307_datetime_t;

// ------------ helper functions--------------
// Helper: BCD to Decimal conversion
uint8_t bcd_to_dec(uint8_t val) {
    return ((val / 16) * 10) + (val % 16);
}

// Helper: Decimal to BCD conversion
uint8_t dec_to_bcd(uint8_t val) {
    return ((val / 10) * 16) + (val % 10);
}

// ------------ I2C helper functions--------------
void ds1307_write_register(uint8_t reg, uint8_t value){
    uint8_t data[2] = {reg, value};
    i2c_write_blocking(I2C_PORT, DS1307_ADDR, data, 2, false);
}

void ds1307_read_register(uint8_t reg, uint8_t *buf, uint8_t len){
    i2c_write_blocking(I2C_PORT, DS1307_ADDR, &reg, 1, true);
    i2c_read_blocking(I2C_PORT, DS1307_ADDR, buf, len, false);
}

// ------------ ds1307 initialization ------------
void ds1307_init(void){
    uint8_t seconds;
    ds1307_read_register(DS1307_REG_SECONDS, &seconds, 1);      // Read seconds register

     // Check if oscillator is disabled (bit 7 = 1)
     if (seconds & 0x80){
        // Enable oscillator by clearing bit 7
        ds1307_write_register(DS1307_REG_SECONDS, seconds & 0x7f);
     }

     //Disable square wave output
     ds1307_write_register(DS1307_REG_CONTROL, 0x00);
}

// ------------ Set date and time ------------
void ds1307_set_datetime( ds1307_datetime_t *dt){
    ds1307_write_register(DS1307_REG_SECONDS, dec_to_bcd(dt->seconds) & 0x7f);
    ds1307_write_register(DS1307_REG_MINUTES, dec_to_bcd(dt->minutes));
    ds1307_write_register(DS1307_REG_HOURS, dec_to_bcd(dt->hours) & 0x3F);
    ds1307_write_register(DS1307_REG_DAY, dec_to_bcd(dt->day));
    ds1307_write_register(DS1307_REG_DATE, dec_to_bcd(dt->date));
    ds1307_write_register(DS1307_REG_MONTH, dec_to_bcd(dt->month));
    ds1307_write_register(DS1307_REG_YEAR, dec_to_bcd(dt->year));
}

// ------------ read current date and time ------------
void ds1307_get_datetime(ds1307_datetime_t *dt){
    uint8_t data[7];
    ds1307_read_register(DS1307_REG_SECONDS, data, 7);
    
    dt->seconds = bcd_to_dec(data[0] & 0x7F);
    dt->minutes = bcd_to_dec(data[1] & 0x7F);
    dt->hours   = bcd_to_dec(data[2] & 0x3F);
    dt->day     = bcd_to_dec(data[3] & 0x07);
    dt->date    = bcd_to_dec(data[4] & 0x3F);
    dt->month   = bcd_to_dec(data[5] & 0x1F);
    dt->year    = bcd_to_dec(data[6]);
}
//  ------------ Print datetime in readable format  ------------
void print_datetime(const ds1307_datetime_t *dt) {
    const char* days[] = {"", "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
    
    printf("Day: %s, 20%02d-%02d-%02d  Time: %02d:%02d:%02d\n",
           days[dt->day], 
           dt->year, dt->month, dt->date,
           dt->hours, dt->minutes, dt->seconds);
}

// ------------ main  ------------
int main(){
    stdio_init_all();

    // Initialize I2C at 100kHz (DS1307 max is 100kHz)
    i2c_init(I2C_PORT, BAUD_RATE);
    gpio_set_function(I2C0_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(I2C0_SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(I2C0_SDA_PIN);
    gpio_pull_up(I2C0_SCL_PIN);

    sleep_ms(2000);

    // Set initial time using struct
    // Uncomment to set time (only needed once)
    ds1307_datetime_t initial_time = {
        .seconds = 0,
        .minutes = 30,
        .hours = 11,      // 5:08 PM
        .day = 6,         // Saturday
        .date = 01,
        .month = 11,
        .year = 25        // 2025
    };
    ds1307_set_datetime(&initial_time);
    printf("Time set!\n\n");
    
    printf("Reading time every second...\n\n");
    
    ds1307_datetime_t current_time;
    
    while (1) {
        // Read current time into struct
        ds1307_get_datetime(&current_time);
        
        // Display time
        print_datetime(&current_time);
        
        sleep_ms(1000);
    }
}