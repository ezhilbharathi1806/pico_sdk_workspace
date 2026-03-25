#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

const int task_delay = 500;
const int task_size = 128;

SemaphoreHandle_t toggle_sem;

void vTaskSMP_demo_delay(void *pvParameters){

    for (;;) {
        xSemaphoreGive(toggle_sem);
        vTaskDelay(task_delay);
    }

}

void vTaskSMP_demo_led(void *pvParameters){

    for (;;) {
        if(xSemaphoreTake(toggle_sem, portMAX_DELAY)){
            gpio_put(25, !gpio_get_out_level(25));
        }
    }
}

void main(){
    stdio_init_all();

    gpio_init(25);
    gpio_set_dir(25,1);

    toggle_sem = xSemaphoreCreateBinary();

    xTaskCreate(vTaskSMP_demo_delay, "A", task_size, NULL, 1, NULL);
    xTaskCreate(vTaskSMP_demo_led, "B", task_size, NULL, 1, NULL);
    vTaskStartScheduler();
}