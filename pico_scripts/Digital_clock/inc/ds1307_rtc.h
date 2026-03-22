#ifndef DS1307_RTC_H
#define DS1307_RTC_H

#include <stdint.h>
#include "hardware/i2c.h"

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

// Structure for 12-hour format display
typedef struct {
    uint8_t hours;      // 1-12
    uint8_t minutes;    // 0-59
    uint8_t seconds;    // 0-59
    bool is_pm;         // false = AM, true = PM
} time_12h_t;

uint8_t bcd_to_dec(uint8_t val);
uint8_t dec_to_bcd(uint8_t val);

void ds1307_write_register(i2c_inst_t *i2c,uint8_t reg, uint8_t value);
void ds1307_read_register(i2c_inst_t *i2c,uint8_t reg, uint8_t *buf, uint8_t len);

void ds1307_init(i2c_inst_t *i2c);
void ds1307_set_datetime( i2c_inst_t *i2c, ds1307_datetime_t *dt);
void ds1307_get_datetime(i2c_inst_t *i2c, ds1307_datetime_t *dt);
void convert_to_12h(const ds1307_datetime_t *dt, time_12h_t *time_12h);

void print_datetime_24h(const ds1307_datetime_t *dt);
void print_datetime_12h(const ds1307_datetime_t *dt);
void print_datetime_both(const ds1307_datetime_t *dt);

#endif