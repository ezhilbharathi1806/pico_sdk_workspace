#include <stdio.h>
#include <stdbool.h>
#include "pico/stdlib.h"

#define button_pin 14       //button pin
#define led_pin 25      //onboard led pin

int main(){

    // Initialize the chosen serial port (for debugging, optional)
    stdio_init_all();

    //initialize gpio
    gpio_init(button_pin);
    gpio_set_dir(button_pin, GPIO_IN);
    gpio_pull_up(button_pin);   // Enable pull-up, assuming button connects to GND when pressed

    gpio_init(led_pin);
    gpio_set_dir(led_pin, GPIO_OUT);
    

    while(1){
        
        if(gpio_get(button_pin) == 0){   //button pressed
            gpio_put(led_pin, true);    //led on
        }
        else{   //button not pressed
            gpio_put(led_pin, false);   //led off
        }   
        sleep_ms(100);  //debounce delay

    }
}