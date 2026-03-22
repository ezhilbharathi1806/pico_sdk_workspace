#include <stdio.h>
#include "pico/stdlib.h"

#define CLK_PIN 0
#define DT_PIN 1
#define SW_PIN 2

int main() {
  stdio_init_all();
  
  gpio_init(CLK_PIN);
  gpio_set_dir(CLK_PIN, GPIO_IN);
  gpio_pull_up(CLK_PIN);
  
  gpio_init(DT_PIN);
  gpio_set_dir(DT_PIN, GPIO_IN);
  gpio_pull_up(DT_PIN);
  
  gpio_init(SW_PIN);
  gpio_set_dir(SW_PIN, GPIO_IN);
  gpio_pull_up(SW_PIN);

  sleep_ms(2000);  // Wait for serial
  printf("====== Rotary Encoder example ======\n");
  
  int previousCLK = gpio_get(CLK_PIN);
  int counter = 0;
  int last_printed_counter = 0;  // Track last printed value
  
  while(1) {
    int currentCLK = gpio_get(CLK_PIN);
    
    if (currentCLK != previousCLK) {
      if (gpio_get(DT_PIN) != currentCLK) {
        counter++;
      } else {
        counter--;
      }
       // Only print if counter actually changed
      if (counter != last_printed_counter) {
        printf("Position: %d\n", counter);
        last_printed_counter = counter;
      }
    previousCLK = currentCLK;
    sleep_us(100);
    }
    
    if(!(gpio_get(SW_PIN))){
      printf("Button presses\n");
      sleep_ms(500);        //Debounce time
    }
  }
}
