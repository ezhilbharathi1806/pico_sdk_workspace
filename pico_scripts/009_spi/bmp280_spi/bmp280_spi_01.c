#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/spi.h"

// SPI Configuration
#define SPI_PORT spi0
#define PIN_MISO 16
#define PIN_CS   17
#define PIN_SCK  18
#define PIN_MOSI 19

// BMP280 Registers
#define BMP280_REG_TEMP_XLSB   0xFC
#define BMP280_REG_TEMP_LSB    0xFB
#define BMP280_REG_TEMP_MSB    0xFA
#define BMP280_REG_PRESS_XLSB  0xF9
#define BMP280_REG_PRESS_LSB   0xF8
#define BMP280_REG_PRESS_MSB   0xF7
#define BMP280_REG_CONFIG      0xF5
#define BMP280_REG_CTRL_MEAS   0xF4
#define BMP280_REG_STATUS      0xF3
#define BMP280_REG_RESET       0xE0
#define BMP280_REG_ID          0xD0

// Calibration registers
#define BMP280_REG_DIG_T1      0x88
#define BMP280_REG_DIG_P1      0x8E

// Calibration data structure
typedef struct {
    uint16_t dig_T1;
    int16_t  dig_T2;
    int16_t  dig_T3;
    uint16_t dig_P1;
    int16_t  dig_P2;
    int16_t  dig_P3;
    int16_t  dig_P4;
    int16_t  dig_P5;
    int16_t  dig_P6;
    int16_t  dig_P7;
    int16_t  dig_P8;
    int16_t  dig_P9;
} bmp280_calib_data;

bmp280_calib_data calib;
int32_t t_fine;

// Chip Select control
static inline void cs_select() {
    asm volatile("nop \n nop \n nop");
    gpio_put(PIN_CS, 0);
    asm volatile("nop \n nop \n nop");
}

static inline void cs_deselect() {
    asm volatile("nop \n nop \n nop");
    gpio_put(PIN_CS, 1);
    asm volatile("nop \n nop \n nop");
}

// Write to BMP280 register
void bmp280_write_register(uint8_t reg, uint8_t data) {
    uint8_t buf[2];
    buf[0] = reg & 0x7F;  // Clear bit 7 for write
    buf[1] = data;
    
    cs_select();
    spi_write_blocking(SPI_PORT, buf, 2);
    cs_deselect();
}

// Read from BMP280 register
void bmp280_read_registers(uint8_t reg, uint8_t *buf, uint16_t len) {
    reg |= 0x80;  // Set bit 7 for read
    
    cs_select();
    spi_write_blocking(SPI_PORT, &reg, 1);
    sleep_ms(1);
    spi_read_blocking(SPI_PORT, 0, buf, len);
    cs_deselect();
}

// Read calibration data
void bmp280_read_calibration() {
    uint8_t buf[24];
    
    // Read temperature calibration
    bmp280_read_registers(BMP280_REG_DIG_T1, buf, 6);
    calib.dig_T1 = (buf[1] << 8) | buf[0];
    calib.dig_T2 = (buf[3] << 8) | buf[2];
    calib.dig_T3 = (buf[5] << 8) | buf[4];
    
    // Read pressure calibration
    bmp280_read_registers(BMP280_REG_DIG_P1, buf, 18);
    calib.dig_P1 = (buf[1] << 8) | buf[0];
    calib.dig_P2 = (buf[3] << 8) | buf[2];
    calib.dig_P3 = (buf[5] << 8) | buf[4];
    calib.dig_P4 = (buf[7] << 8) | buf[6];
    calib.dig_P5 = (buf[9] << 8) | buf[8];
    calib.dig_P6 = (buf[11] << 8) | buf[10];
    calib.dig_P7 = (buf[13] << 8) | buf[12];
    calib.dig_P8 = (buf[15] << 8) | buf[14];
    calib.dig_P9 = (buf[17] << 8) | buf[16];
}

// Initialize BMP280
void bmp280_init() {
    // Read and verify chip ID (should be 0x58)
    uint8_t chip_id;
    bmp280_read_registers(BMP280_REG_ID, &chip_id, 1);
    printf("BMP280 Chip ID: 0x%02X (expected 0x58)\n", chip_id);
    
    // Soft reset
    bmp280_write_register(BMP280_REG_RESET, 0xB6);
    sleep_ms(100);
    
    // Read calibration data
    bmp280_read_calibration();
    
    // Configure: standby 500ms, filter off, SPI 4-wire mode
    bmp280_write_register(BMP280_REG_CONFIG, 0xA0);
    
    // Control: temp oversampling x2, pressure oversampling x16, normal mode
    bmp280_write_register(BMP280_REG_CTRL_MEAS, 0x57);
    
    sleep_ms(100);
}

// Compensate temperature (returns in degrees Celsius)
int32_t bmp280_compensate_temp(int32_t adc_T) {
    int32_t var1, var2, T;
    
    var1 = ((((adc_T >> 3) - ((int32_t)calib.dig_T1 << 1))) * 
            ((int32_t)calib.dig_T2)) >> 11;
    var2 = (((((adc_T >> 4) - ((int32_t)calib.dig_T1)) * 
              ((adc_T >> 4) - ((int32_t)calib.dig_T1))) >> 12) * 
            ((int32_t)calib.dig_T3)) >> 14;
    
    t_fine = var1 + var2;
    T = (t_fine * 5 + 128) >> 8;
    return T;
}

// Compensate pressure (returns in Pa)
uint32_t bmp280_compensate_pressure(int32_t adc_P) {
    int64_t var1, var2, p;
    
    var1 = ((int64_t)t_fine) - 128000;
    var2 = var1 * var1 * (int64_t)calib.dig_P6;
    var2 = var2 + ((var1 * (int64_t)calib.dig_P5) << 17);
    var2 = var2 + (((int64_t)calib.dig_P4) << 35);
    var1 = ((var1 * var1 * (int64_t)calib.dig_P3) >> 8) + 
           ((var1 * (int64_t)calib.dig_P2) << 12);
    var1 = (((((int64_t)1) << 47) + var1)) * ((int64_t)calib.dig_P1) >> 33;
    
    if (var1 == 0) {
        return 0;
    }
    
    p = 1048576 - adc_P;
    p = (((p << 31) - var2) * 3125) / var1;
    var1 = (((int64_t)calib.dig_P9) * (p >> 13) * (p >> 13)) >> 25;
    var2 = (((int64_t)calib.dig_P8) * p) >> 19;
    p = ((p + var1 + var2) >> 8) + (((int64_t)calib.dig_P7) << 4);
    
    return (uint32_t)p;
}

// Read temperature and pressure
void bmp280_read_data(float *temperature, float *pressure) {
    uint8_t buf[6];
    
    // Read all data registers
    bmp280_read_registers(BMP280_REG_PRESS_MSB, buf, 6);
    
    // Combine pressure bytes (20-bit)
    int32_t adc_P = ((buf[0] << 16) | (buf[1] << 8) | buf[2]) >> 4;
    
    // Combine temperature bytes (20-bit)
    int32_t adc_T = ((buf[3] << 16) | (buf[4] << 8) | buf[5]) >> 4;
    
    // Compensate temperature
    int32_t temp = bmp280_compensate_temp(adc_T);
    *temperature = temp / 100.0f;
    
    // Compensate pressure
    uint32_t press = bmp280_compensate_pressure(adc_P);
    *pressure = press / 256.0f / 100.0f;  // Convert to hPa
}

int main() {
    stdio_init_all();
    
    // Initialize SPI at 1 MHz (BMP280 supports up to 10 MHz)
    spi_init(SPI_PORT, 1000 * 1000);
    gpio_set_function(PIN_MISO, GPIO_FUNC_SPI);
    gpio_set_function(PIN_SCK, GPIO_FUNC_SPI);
    gpio_set_function(PIN_MOSI, GPIO_FUNC_SPI);
    
    // Initialize CS pin
    gpio_init(PIN_CS);
    gpio_set_dir(PIN_CS, GPIO_OUT);
    gpio_put(PIN_CS, 1);
    
    sleep_ms(2000);  // Wait for serial console
    
    printf("BMP280 SPI Example\n");
    printf("==================\n\n");
    
    // Initialize BMP280
    bmp280_init();
    
    printf("Reading temperature and pressure...\n\n");
    
    while (1) {
        float temperature, pressure;
        bmp280_read_data(&temperature, &pressure);
        
        printf("Temperature: %.2f °C  |  Pressure: %.2f hPa\n", 
               temperature, pressure);
        
        sleep_ms(1000);
    }
    
    return 0;
}
