#include "buzzer.h"
#include "pico/stdlib.h"


void buzzer_init(void){
    gpio_init(BUZZER_PIN);
    gpio_set_dir(BUZZER_PIN, GPIO_OUT);
    gpio_put(BUZZER_PIN, 0);  // Ensure it starts off
}

void buzzer_on(void){
    gpio_put(BUZZER_PIN, 1);
}

void buzzer_off(void){
    gpio_put(BUZZER_PIN, 0);
}

void buzzer_beep(uint8_t num){
    // Validate input
    if (num == 0) return;
    if (num > 10) num = 10;  // Limit to 10 beeps max
    
    for (uint8_t i = 0; i < num; i++){
        gpio_put(BUZZER_PIN, 1);
        sleep_ms(200);
        gpio_put(BUZZER_PIN, 0);
        sleep_ms(200);
    }
}
