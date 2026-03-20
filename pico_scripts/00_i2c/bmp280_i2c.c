
#include <stdio.h>
#include <math.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"

// I2C configuration
#define I2C_PORT    i2c0
#define I2C0_SDA_PIN    4
#define I2C0_SCL_PIN    5
#define BAUD_RATE 400000
#define BMP280_ADDR 0x76        // default I2C address

// BMP280 registers
#define REG_ID 0xD0
#define REG_RESET 0xE0
#define REG_STATUS 0xF3
#define REG_CTRL_MEAS 0xF4
#define REG_CONFIG 0xF5
#define REG_PRESS_MSB 0xF7
#define REG_TEMP_MSB 0xFA
#define REG_CALIB00 0x88

//Calibration data structure
struct bmp280_calib_param{
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
};

struct bmp280_calib_param calib_data;
int32_t t_fine;

// --------------- Helper: Read and Write ---------------
void bmp280_write_register(uint8_t reg, uint8_t value){
    uint8_t data[2] = {reg, value};
    i2c_write_blocking(I2C_PORT, BMP280_ADDR, data, 2, false);
}
void bmp280_read_register(uint8_t reg, uint8_t *buf, uint8_t len){
    i2c_write_blocking(I2C_PORT, BMP280_ADDR, &reg, 1, true);
    i2c_read_blocking(I2C_PORT, BMP280_ADDR, buf, len, false);
}

// --------------- Compensation Functions ---------------
int32_t bmp280_compensate_temp(int32_t adc_T) {
    int32_t var1, var2, T;
    var1 = ((((adc_T >> 3) - ((int32_t)calib_data.dig_T1 << 1))) *
            ((int32_t)calib_data.dig_T2)) >> 11;
    var2 = (((((adc_T >> 4) - ((int32_t)calib_data.dig_T1)) *
              ((adc_T >> 4) - ((int32_t)calib_data.dig_T1))) >> 12) *
            ((int32_t)calib_data.dig_T3)) >> 14;
    t_fine = var1 + var2;
    T = (t_fine * 5 + 128) >> 8;
    return T; // Temperature in °C *100
}

uint32_t bmp280_compensate_pressure(int32_t adc_P) {
    int64_t var1, var2, p;
    var1 = ((int64_t)t_fine) - 128000;
    var2 = var1 * var1 * (int64_t)calib_data.dig_P6;
    var2 = var2 + ((var1 * (int64_t)calib_data.dig_P5) << 17);
    var2 = var2 + (((int64_t)calib_data.dig_P4) << 35);
    var1 = ((var1 * var1 * (int64_t)calib_data.dig_P3) >> 8) +
           ((var1 * (int64_t)calib_data.dig_P2) << 12);
    var1 =
        (((((int64_t)1) << 47) + var1)) * ((int64_t)calib_data.dig_P1) >> 33;

    if (var1 == 0) return 0; // avoid div by zero

    p = 1048576 - adc_P;
    p = (((p << 31) - var2) * 3125) / var1;
    var1 = (((int64_t)calib_data.dig_P9) * (p >> 13) * (p >> 13)) >> 25;
    var2 = (((int64_t)calib_data.dig_P8) * p) >> 19;
    p = ((p + var1 + var2) >> 8) + (((int64_t)calib_data.dig_P7) << 4);
    return (uint32_t)p; // Pa
}

// --------------- Initialize BMP280 ---------------
void bmp280_init(void){
    uint8_t buf[24];
    bmp280_read_register(REG_CALIB00, buf, 24);

    calib_data.dig_T1 = (buf[1] << 8) | buf[0];
    calib_data.dig_T2 = (buf[3] << 8) | buf[2];
    calib_data.dig_T3 = (buf[5] << 8) | buf[4];
    calib_data.dig_P1 = (buf[7] << 8) | buf[6];
    calib_data.dig_P2 = (buf[9] << 8) | buf[8];
    calib_data.dig_P3 = (buf[11] << 8) | buf[10];
    calib_data.dig_P4 = (buf[13] << 8) | buf[12];
    calib_data.dig_P5 = (buf[15] << 8) | buf[14];
    calib_data.dig_P6 = (buf[17] << 8) | buf[16];
    calib_data.dig_P7 = (buf[19] << 8) | buf[18];
    calib_data.dig_P8 = (buf[21] << 8) | buf[20];
    calib_data.dig_P9 = (buf[23] << 8) | buf[22];

    bmp280_write_register(REG_CTRL_MEAS, 0x37);     // Normal mode, 1x oversampling temperature, 16x oversampling pressure
    bmp280_write_register(REG_CONFIG, 0xA0);        // 1000ms standby, filter x4
}

// --------------- Read raw data ---------------
void bmp280_read_raw(uint32_t *temp, uint32_t *press){
    uint8_t data[6];
    bmp280_read_register(REG_PRESS_MSB, data, 6);

    *press = (int32_t)((uint32_t)(data[0]<<12) | (uint32_t)(data[1]<<4) | (uint32_t)(data[2]>>4));
    *temp =  (int32_t)((uint32_t)(data[3]<<12) | (uint32_t)(data[4]<<4) | (uint32_t)(data[5]>>4));
}

// --------------- calculate altitude ---------------
float calculate_pressure(float press_pa){
    return 44330.0 * (1.0 - pow((press_pa/101325),0.1903));        //Reference sea-level pressure	1013.25 hPa or 101325 Pa
}
// --------------- Main function ---------------
int main(){
    stdio_init_all();         //initialize standard IO (UART/USB)

    // Initialize I2C at defined baud rate and pins
    i2c_init(I2C_PORT, BAUD_RATE);
    gpio_set_function(I2C0_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(I2C0_SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(I2C0_SDA_PIN);
    gpio_pull_up(I2C0_SCL_PIN);

    bmp280_init();

    printf("BMP280 I2C device initialized\n");
    while(1){
        int32_t raw_temp, raw_press;
        bmp280_read_raw(&raw_temp, &raw_press);

        printf("%d\t",raw_temp);
        printf("%d\n",raw_press);

        int32_t temp = bmp280_compensate_temp(raw_temp);
        uint32_t press = bmp280_compensate_pressure(raw_press);

        printf("Temperature: %.2f °C | Pressure: %.2f Pa\n", temp / 100.0, press / 256.0);

        printf("Pressure in meters: %f m\n", calculate_pressure(press/256.0));

        sleep_ms(1000);
    }
    return 0;
}