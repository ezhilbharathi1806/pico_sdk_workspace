#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"

// I2C configuration
#define I2C_PORT    i2c0
#define I2C0_SDA_PIN    4
#define I2C0_SCL_PIN    5
#define BAUD_RATE 400000
#define MPU6050_ADDR 0x68        // default I2C address

// MPU6050 registers
#define PWR_MGMT_1  0x6B
#define CONFIG      0x1A
#define GYRO_CONFIG  0x1B
#define ACCEL_CONFIG 0x1C
#define ACCEL_XOUT_H 0x3B
// #define GYRO_XOUT_H  0x43
// #define TEMP_OUT_H   0x41

// Calibration offsets (computed during calibration phase)
float accel_offset[3] = {0, 0, 0};
float gyro_offset[3] = {0, 0, 0};

// --------------- Helper: Read and Write ---------------
void mpu6050_write_register(uint8_t reg, uint8_t value){
    uint8_t data[2] = {reg, value};
    i2c_write_blocking(I2C_PORT, MPU6050_ADDR, data, 2, false);
}
void mpu6050_read_register(uint8_t reg, uint8_t *buf, uint8_t len){
    i2c_write_blocking(I2C_PORT, MPU6050_ADDR, &reg, 1, true);
    i2c_read_blocking(I2C_PORT, MPU6050_ADDR, buf, len, false);
}

void mpu6050_init() {
    // Wake up device (clear sleep bit)
    mpu6050_write_register(PWR_MGMT_1, 0x00);
    sleep_ms(100);

    // Configure DLPF (Digital Low-Pass Filter) - 44 Hz bandwidth
    mpu6050_write_register(CONFIG, 0x03);
    
    // Set gyroscope range: ±250°/s (most sensitive)
    mpu6050_write_register(GYRO_CONFIG, 0x00);
    
    // Set accelerometer range: ±2g (most sensitive)
    mpu6050_write_register(ACCEL_CONFIG, 0x00);
    
    sleep_ms(100);
}

void mpu6050_calibrate(int num_samples) {
    printf("Calibrating... Keep sensor still!\n");
    
    float accel_sum[3] = {0, 0, 0};
    float gyro_sum[3] = {0, 0, 0};
    
    for (int i = 0; i < num_samples; i++) {
        uint8_t data[14];
        mpu6050_read_register(ACCEL_XOUT_H, data, 14);
        
        // Correct byte order: HIGH byte first
        int16_t ax = (data[0] << 8) | data[1];
        int16_t ay = (data[2] << 8) | data[3];
        int16_t az = (data[4] << 8) | data[5];
        int16_t gx = (data[8] << 8) | data[9];
        int16_t gy = (data[10] << 8) | data[11];
        int16_t gz = (data[12] << 8) | data[13];
        
        accel_sum[0] += ax / 16384.0;
        accel_sum[1] += ay / 16384.0;
        accel_sum[2] += az / 16384.0;
        
        gyro_sum[0] += gx / 131.0;
        gyro_sum[1] += gy / 131.0;
        gyro_sum[2] += gz / 131.0;
        
        sleep_ms(5);
    }
    
    // Calculate average offsets
    for (int i = 0; i < 3; i++) {
        accel_offset[i] = accel_sum[i] / num_samples;
        gyro_offset[i] = gyro_sum[i] / num_samples;
    }
    
    // Z-axis should read ~1g due to gravity, subtract 1.0
    accel_offset[2] -= 1.0;
    
    printf("Calibration complete!\n");
    printf("Accel offsets: X=%.3f Y=%.3f Z=%.3f\n", 
           accel_offset[0], accel_offset[1], accel_offset[2]);
    printf("Gyro offsets: X=%.3f Y=%.3f Z=%.3f\n\n", 
           gyro_offset[0], gyro_offset[1], gyro_offset[2]);
}

int main(void){
    stdio_init_all();         //initialize standard IO (UART/USB)

    // Initialize I2C at defined baud rate and pins
    i2c_init(I2C_PORT, BAUD_RATE);
    gpio_set_function(I2C0_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(I2C0_SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(I2C0_SDA_PIN);
    gpio_pull_up(I2C0_SCL_PIN);

    mpu6050_init();

    // Calibrate with 500 samples (takes ~2.5 seconds)
    mpu6050_calibrate(500);

    while(1){
        uint8_t data[14];

        mpu6050_read_register(ACCEL_XOUT_H, data, 14);

        // Correct byte order: HIGH byte first
        int16_t ax_raw = (data[0] << 8) | data[1];
        int16_t ay_raw = (data[2] << 8) | data[3];
        int16_t az_raw = (data[4] << 8) | data[5];
        int16_t temp_raw = (data[6] << 8) | data[7];
        int16_t gx_raw = (data[8] << 8) | data[9];
        int16_t gy_raw = (data[10] << 8) | data[11];
        int16_t gz_raw = (data[12] << 8) | data[13];
        
        // Convert and apply calibration offsets
        float ax = (ax_raw / 16384.0) - accel_offset[0];
        float ay = (ay_raw / 16384.0) - accel_offset[1];
        float az = (az_raw / 16384.0) - accel_offset[2];
        
        float gx = (gx_raw / 131.0) - gyro_offset[0];
        float gy = (gy_raw / 131.0) - gyro_offset[1];
        float gz = (gz_raw / 131.0) - gyro_offset[2];
        
        float temperature = (temp_raw / 340.0) + 36.53;
        
        printf("Accel [g]: X=%6.3f Y=%6.3f Z=%6.3f | Gyro [deg/s]: X=%6.2f Y=%6.2f Z=%6.2f | Temp=%.2f°C\n",
               ax, ay, az, gx, gy, gz, temperature);

        sleep_ms(500);      //500ms delay between readings

    }

}