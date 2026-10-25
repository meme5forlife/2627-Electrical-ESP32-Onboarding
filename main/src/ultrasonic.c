#include "ultrasonic.h"
#include "driver/gpio.h"
#include "driver/uart.h"
#include "esp_timer.h"
#include "esp_rom_sys.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// Define which UART port your training robot is using (e.g., UART_NUM_1)
#define ULTRASONIC_UART_PORT UART_NUM_1 

Ultrasonic init_ultrasonic(uint8_t pT, uint8_t pE){
    // Configures trigger pin as GPIO output and echo pin as GPIO input
    // Returns Ultrasonic struct with pinTrig and pinEcho initialized
    // 1. reset pins
    gpio_reset_pin(pT);
    gpio_reset_pin(pE);

    // 2. set directions
    gpio_set_direction(pT, GPIO_MODE_OUTPUT);
    gpio_set_direction(pE, GPIO_MODE_INPUT);

    // 3. set initial trigger level to low
    gpio_set_level(pT, 0);

    // 4. create and return the sensor
    Ultrasonic sensor = {
        .pinTrig = pT,
        .pinEcho = pE
    };
    return sensor;
}

uint32_t pulse_in(uint8_t pin, bool state){
    // Define 1 second timeout (1,000,000 microseconds) to prevent lockups
    // Wait for any pre-existing pulse of the target state to end
    // Wait for the target pulse state to start
    // Capture the start time for the pulse
    // Wait for the target pulse state to end
    // Calculate and return total elapsed time in microseconds, or return 0 if timeout occurs
    // define timeout (1 second in microseconds)
    uint32_t timeout = 1000000; 

    // wait for any pre-existing pulse of the target state to end
    while (gpio_get_level(pin) == state) {
        if (timeout-- == 0) return 0; // timeout occurred
        esp_rom_delay_us(1);          // small delay per loop iteration
    }

    // reset timeout for the next wait phase
    timeout = 1000000;

    // wait for the target pulse state to start
    while (gpio_get_level(pin) != state) {
        if (timeout-- == 0) return 0;
        esp_rom_delay_us(1);
    }

    // 4. Capture the start time for the pulse
    int start_time = esp_timer_get_time();

    // Reset timeout for the final wait phase
    timeout = 1000000;

    // 5. Wait for the target pulse state to end
    while (gpio_get_level(pin) == state) {
        if (timeout-- == 0) return 0;
        esp_rom_delay_us(1);
    }

    // 6. Capture end time and calculate total elapsed time
    int end_time = esp_timer_get_time();
    return (uint32_t)(end_time - start_time);
}   

double calculate_distance(Ultrasonic *ultrasonic){
    // Define duration and distance variables
    // Trigger ultrasonic burst
    // Calculate and return distance
    
    gpio_set_level(ultrasonic->pinTrig, 1);//trigger ultrasonic burst
    esp_rom_delay_us(10);
    gpio_set_level(ultrasonic->pinTrig, 0);

    uint32_t duration = pulse_in(ultrasonic->pinEcho, true);//define duration var

    double distance = (duration * 0.0343)/2;//define and calculate distance var
    return distance;// returns distance var
}