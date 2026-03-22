#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/pwm.h"

#define SERVO_PIN 0

void set_servo_pulse_us(uint gpio, uint pulse_us) {
    uint slice = pwm_gpio_to_slice_num(gpio);
    // 20000us period, calculate level
    uint16_t level = (uint16_t)((pulse_us * 65535) / 20000);
    pwm_set_gpio_level(gpio, level);
}

int main() {
    gpio_set_function(SERVO_PIN, GPIO_FUNC_PWM);
    uint slice = pwm_gpio_to_slice_num(SERVO_PIN);
    
    pwm_config config = pwm_get_default_config();
    pwm_config_set_clkdiv(&config, 64.0f);
    pwm_config_set_wrap(&config, 39061);
    pwm_init(slice, &config, true);
    
    while(1) {
        set_servo_pulse_us(SERVO_PIN, 1000);  // 0°
        sleep_ms(1000);
        set_servo_pulse_us(SERVO_PIN, 1500);  // 90°
        sleep_ms(1000);
        set_servo_pulse_us(SERVO_PIN, 2000);  // 180°
        sleep_ms(1000);
    }
}
