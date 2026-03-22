#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/pwm.h"

#define SERVO_PIN 16  // GPIO 16 (PWM0 A)

// Servo timing constants for 50Hz (20ms period)
#define SERVO_FREQ_HZ 50
#define SERVO_MIN_PULSE_US 1000  // 1ms for 0 degrees
#define SERVO_MAX_PULSE_US 2000  // 2ms for 180 degrees
#define SERVO_MID_PULSE_US 1500  // 1.5ms for 90 degrees

// Function to set servo angle (0-180 degrees)
void servo_set_angle(uint gpio, float angle) {
    // Clamp angle to valid range
    if (angle < 0) angle = 0;
    if (angle > 180) angle = 180;
    
    // Calculate pulse width in microseconds
    float pulse_us = SERVO_MIN_PULSE_US + 
                     (angle / 180.0f) * (SERVO_MAX_PULSE_US - SERVO_MIN_PULSE_US);
    
    // Get PWM slice and calculate duty cycle
    uint slice_num = pwm_gpio_to_slice_num(gpio);
    uint16_t wrap = pwm_hw->slice[slice_num].top;
    
    // Convert pulse width to PWM level
    // Period = 20ms = 20000us
    uint16_t level = (uint16_t)((pulse_us / 20000.0f) * (float)(wrap + 1));
    
    pwm_set_gpio_level(gpio, level);
}

// Initialize PWM for servo control
void servo_init(uint gpio) {
    // Set GPIO function to PWM
    gpio_set_function(gpio, GPIO_FUNC_PWM);
    
    // Get PWM slice number
    uint slice_num = pwm_gpio_to_slice_num(gpio);
    
    // Get default PWM config
    pwm_config config = pwm_get_default_config();
    
    // Calculate wrap value for 50Hz
    // System clock = 125MHz
    // For 50Hz: period = 20ms = 0.02s
    // wrap = (125MHz / 50Hz) - 1 = 2,500,000 - 1
    // But this exceeds 16-bit max (65535), so we need a divider
    
    // Using clock divider = 64
    // wrap = (125MHz / 64 / 50Hz) - 1 = 39062 - 1 = 39061
    // This gives us good resolution for servo control
    
    float clock_div = 64.0f;
    uint16_t wrap = 39061;
    
    pwm_config_set_clkdiv(&config, clock_div);
    pwm_config_set_wrap(&config, wrap);
    
    // Initialize PWM with config
    pwm_init(slice_num, &config, true);
}

int main() {
    stdio_init_all();
    
    // Initialize servo on GPIO 16
    servo_init(SERVO_PIN);
    
    sleep_ms(2000);
    printf("====== Servo Motor Control Example ======\n");
    
    while(1) {
        // Move to 0 degrees
        printf("Servo: 0 degrees\n");
        servo_set_angle(SERVO_PIN, 0);
        sleep_ms(1000);
        
        // Move to 45 degrees
        printf("Servo: 45 degrees\n");
        servo_set_angle(SERVO_PIN, 45);
        sleep_ms(1000);
        
        // Move to 90 degrees (center)
        printf("Servo: 90 degrees\n");
        servo_set_angle(SERVO_PIN, 90);
        sleep_ms(1000);
        
        // Move to 135 degrees
        printf("Servo: 135 degrees\n");
        servo_set_angle(SERVO_PIN, 135);
        sleep_ms(1000);
        
        // Move to 180 degrees
        printf("Servo: 180 degrees\n");
        servo_set_angle(SERVO_PIN, 180);
        sleep_ms(1000);
        
        // Back to center
        printf("Servo: 90 degrees (center)\n");
        servo_set_angle(SERVO_PIN, 90);
        sleep_ms(2000);
    }
    
    return 0;
}
