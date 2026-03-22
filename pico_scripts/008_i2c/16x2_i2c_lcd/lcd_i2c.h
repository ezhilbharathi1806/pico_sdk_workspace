#ifndef LCD_I2C_H
#define LCD_I2C_H

#include <stdint.h>
#include "hardware/i2c.h"

// LCD I2C address (typically 0x27 or 0x3F - use i2c_scanner to find yours)
#define LCD_ADDR 0x27

// LCD Commands
#define LCD_CLEAR_DISPLAY   0x01
#define LCD_RETURN_HOME     0x02
#define LCD_ENTRY_MODE_SET  0x04
#define LCD_DISPLAY_CONTROL 0x08
#define LCD_CURSOR_SHIFT    0x10
#define LCD_FUNCTION_SET    0x20
#define LCD_SET_CGRAM_ADDR  0x40
#define LCD_SET_DDRAM_ADDR  0x80

// Flags for display entry mode
#define LCD_ENTRY_RIGHT          0x00
#define LCD_ENTRY_LEFT           0x02
#define LCD_ENTRY_SHIFT_INCREMENT 0x01
#define LCD_ENTRY_SHIFT_DECREMENT 0x00

// Flags for display on/off control
#define LCD_DISPLAY_ON  0x04
#define LCD_DISPLAY_OFF 0x00
#define LCD_CURSOR_ON   0x02
#define LCD_CURSOR_OFF  0x00
#define LCD_BLINK_ON    0x01
#define LCD_BLINK_OFF   0x00

// Flags for function set
#define LCD_8BIT_MODE 0x10
#define LCD_4BIT_MODE 0x00
#define LCD_2_LINE    0x08
#define LCD_1_LINE    0x00
#define LCD_5x10_DOTS 0x04
#define LCD_5x8_DOTS  0x00

// Backlight control
#define LCD_BACKLIGHT   0x08
#define LCD_NO_BACKLIGHT 0x00

// Enable bit
#define LCD_ENABLE 0x04

// Mode - RS bit
#define LCD_MODE_COMMAND 0x00
#define LCD_MODE_DATA    0x01

// LCD dimensions
#define LCD_ROWS 2
#define LCD_COLS 16

// Function prototypes
void lcd_init(i2c_inst_t *i2c, uint8_t addr);
void lcd_clear(void);
void lcd_home(void);
void lcd_set_cursor(uint8_t row, uint8_t col);
void lcd_print(const char *str);
void lcd_print_char(char c);
void lcd_backlight_on(void);
void lcd_backlight_off(void);
void lcd_display_on(void);
void lcd_display_off(void);
void lcd_cursor_on(void);
void lcd_cursor_off(void);
void lcd_blink_on(void);
void lcd_blink_off(void);

#endif // LCD_I2C_H
