#ifndef BUZZER_H
#define BUZZER_H

#include <stdint.h>
#define BUZZER_PIN 0

void buzzer_init(void);
void buzzer_on(void);
void buzzer_off(void);
void buzzer_beep(uint8_t num);
#endif