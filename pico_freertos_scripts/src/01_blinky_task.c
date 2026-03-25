#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"

#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include "task.h"

#define gLED_PIN 25
#define wLED_PIN 0

void greenLedTask(void *params){     //Task - 1
    while(1){
        gpio_put(gLED_PIN, 1);
        vTaskDelay(pdMS_TO_TICKS(1000));
        gpio_put(gLED_PIN, 0);
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void whiteLEDTask(void *param){       //Task - 2
    while(1){
        gpio_put(wLED_PIN, 1);
        vTaskDelay(pdMS_TO_TICKS(500));
        gpio_put(wLED_PIN, 0);
        vTaskDelay(pdMS_TO_TICKS(300));
    }
}

int main(void){
    stdio_init_all();

    gpio_init(gLED_PIN);
    gpio_set_dir(gLED_PIN, GPIO_OUT);

    gpio_init(wLED_PIN);
    gpio_set_dir(wLED_PIN, GPIO_OUT);

    TaskHandle_t gLedTask = NULL;
    TaskHandle_t wLedTask = NULL;

    BaseType_t status1 = xTaskCreate(greenLedTask, "Green Led", 256, NULL, tskIDLE_PRIORITY, &gLedTask);

    BaseType_t status2 = xTaskCreate(whiteLEDTask, "white Led", 256, NULL, tskIDLE_PRIORITY, &wLedTask);

    vTaskStartScheduler();

    while(1){
        //should never get here
    }
}