#include <stdio.h>
#include "pico/stdlib.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#define LED_PIN 25

// Task 1: Blink LED
void led_task(void *pvParameters) {
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);
    
    while (1) {
        gpio_put(LED_PIN, 1);
        vTaskDelay(pdMS_TO_TICKS(500));
        gpio_put(LED_PIN, 0);
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

// Task 2: Print counter
void print_task(void *pvParameters) {
    int counter = 0;
    
    while (1) {
        printf("Counter: %d\n", counter++);
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

// Task 3: High priority task
void high_priority_task(void *pvParameters) {
    while (1) {
        printf("[High Priority] Running\n");
        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}

int main() {
    stdio_init_all();
    
    // Small delay for USB enumeration
    sleep_ms(2000);
    
    printf("FreeRTOS Example on Raspberry Pi Pico\n");
    printf("======================================\n\n");
    
    // Create tasks
    xTaskCreate(led_task, "LED_Task", 256, NULL, 1, NULL);
    xTaskCreate(print_task, "Print_Task", 512, NULL, 1, NULL);
    xTaskCreate(high_priority_task, "High_Pri", 512, NULL, 2, NULL);
    
    // Start the scheduler
    vTaskStartScheduler();
    
    // Should never reach here
    while (1) {
        tight_loop_contents();
    }
    
    return 0;
}

// FreeRTOS malloc failed hook
void vApplicationMallocFailedHook(void) {
    printf("ERROR: Malloc failed!\n");
    while(1);
}

// FreeRTOS stack overflow hook
void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName) {
    printf("ERROR: Stack overflow in task %s\n", pcTaskName);
    while(1);
}
