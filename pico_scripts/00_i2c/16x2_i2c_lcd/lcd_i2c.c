//Each data byte sent over I²C controls all 8 pins of the PCF8574 simultaneously.
//[  D7  D6  D5  D4  BL  EN  RW  RS  ]

#include "lcd_i2c.h"
#include "pico/stdlib.h"

// Private variables
static i2c_inst_t *lcd_i2c_port;
static uint8_t lcd_i2c_addr;
static uint8_t backlight_state = LCD_BACKLIGHT;

// Write byte to I2C
static void i2c_write_byte(uint8_t val) {
    i2c_write_blocking(lcd_i2c_port, lcd_i2c_addr, &val, 1, false);
}

// Send nibble (4 bits) to LCD
static void lcd_send_nibble(uint8_t nibble, uint8_t mode) {
    uint8_t data = (nibble & 0xF0) | mode | backlight_state | LCD_ENABLE;
    i2c_write_byte(data);
    sleep_us(1);
    i2c_write_byte(data & ~LCD_ENABLE);
    sleep_us(10);
}

// Send byte to LCD (as two nibbles)
static void lcd_send_byte(uint8_t val, uint8_t mode) {
    lcd_send_nibble(val & 0xF0, mode);        // High nibble
    lcd_send_nibble((val << 4) & 0xF0, mode); // Low nibble
}

// Send command to LCD
static void lcd_send_command(uint8_t cmd) {
    lcd_send_byte(cmd, LCD_MODE_COMMAND);
}

// Send data to LCD
static void lcd_send_data(uint8_t data) {
    lcd_send_byte(data, LCD_MODE_DATA);
}

// Initialize LCD
void lcd_init(i2c_inst_t *i2c, uint8_t addr) {
    lcd_i2c_port = i2c;
    lcd_i2c_addr = addr;
    
    sleep_ms(50);  // Wait for LCD to power up
    
    // Initialize in 4-bit mode
    lcd_send_nibble(0x30, LCD_MODE_COMMAND);
    sleep_ms(5);
    lcd_send_nibble(0x30, LCD_MODE_COMMAND);
    sleep_us(150);
    lcd_send_nibble(0x30, LCD_MODE_COMMAND);
    sleep_us(150);
    lcd_send_nibble(0x20, LCD_MODE_COMMAND);  // Switch to 4-bit mode
    sleep_us(150);
    
    // Function set: 4-bit mode, 2 lines, 5x8 dots
    lcd_send_command(LCD_FUNCTION_SET | LCD_4BIT_MODE | LCD_2_LINE | LCD_5x8_DOTS);
    
    // Display control: display on, cursor off, blink off
    lcd_send_command(LCD_DISPLAY_CONTROL | LCD_DISPLAY_ON | LCD_CURSOR_OFF | LCD_BLINK_OFF);
    
    // Clear display
    lcd_clear();
    
    // Entry mode: left to right, no shift
    lcd_send_command(LCD_ENTRY_MODE_SET | LCD_ENTRY_LEFT | LCD_ENTRY_SHIFT_DECREMENT);
    
    sleep_ms(2);
}

// Clear display
void lcd_clear(void) {
    lcd_send_command(LCD_CLEAR_DISPLAY);
    sleep_ms(2);
}

// Return cursor to home
void lcd_home(void) {
    lcd_send_command(LCD_RETURN_HOME);
    sleep_ms(2);
}

// Set cursor position (row: 0-1, col: 0-15)
void lcd_set_cursor(uint8_t row, uint8_t col) {
    uint8_t row_offsets[] = {0x00, 0x40};
    if (row >= LCD_ROWS) {
        row = LCD_ROWS - 1;
    }
    if (col >= LCD_COLS) {
        col = LCD_COLS - 1;
    }
    lcd_send_command(LCD_SET_DDRAM_ADDR | (col + row_offsets[row]));
}

// Print string
void lcd_print(const char *str) {
    while (*str) {
        lcd_send_data(*str++);
    }
}

// Print single character
void lcd_print_char(char c) {
    lcd_send_data(c);
}

// Backlight control
void lcd_backlight_on(void) {
    backlight_state = LCD_BACKLIGHT;
    i2c_write_byte(backlight_state);
}

void lcd_backlight_off(void) {
    backlight_state = LCD_NO_BACKLIGHT;
    i2c_write_byte(backlight_state);
}

// Display control
void lcd_display_on(void) {
    lcd_send_command(LCD_DISPLAY_CONTROL | LCD_DISPLAY_ON);
}

void lcd_display_off(void) {
    lcd_send_command(LCD_DISPLAY_CONTROL | LCD_DISPLAY_OFF);
}

// Cursor control
void lcd_cursor_on(void) {
    lcd_send_command(LCD_DISPLAY_CONTROL | LCD_DISPLAY_ON | LCD_CURSOR_ON);
}

void lcd_cursor_off(void) {
    lcd_send_command(LCD_DISPLAY_CONTROL | LCD_DISPLAY_ON | LCD_CURSOR_OFF);
}

// Blink control
void lcd_blink_on(void) {
    lcd_send_command(LCD_DISPLAY_CONTROL | LCD_DISPLAY_ON | LCD_BLINK_ON);
}

void lcd_blink_off(void) {
    lcd_send_command(LCD_DISPLAY_CONTROL | LCD_DISPLAY_ON | LCD_BLINK_OFF);
}
