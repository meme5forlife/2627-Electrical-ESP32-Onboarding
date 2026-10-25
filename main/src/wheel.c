#include "wheel.h"
#include "driver/gpio.h"
#include "driver/ledc.h"

// Define the PWM properties (adjust speed/resolution as needed)
#define MOTOR_PWM_SPEED_MODE   LEDC_LOW_SPEED_MODE
#define MOTOR_PWM_DUTY_RES     LEDC_TIMER_8_BIT      // 0 to 255 range to match your uint8_t duty
#define MOTOR_PWM_FREQ         5000   

void init_hardware_pwm_timer(void){
    // Initializes LEDC timer for PWM control (ledc_timer_config_t)
    //create timer config
    ledc_timer_config_t ledc_timer = {
        .speed_mode       = LEDC_LOW_SPEED_MODE,
        .timer_num        = LEDC_TIMER_0,
        .duty_resolution  = LEDC_TIMER_8_BIT,
        .freq_hz          = 5000,              
        .clk_cfg          = LEDC_AUTO_CLK
    };

    ESP_ERROR_CHECK(ledc_timer_config(&ledc_timer)); 

    ledc_channel_config_t ledc_channel = {
        .gpio_num       = 16,
        .speed_mode     = LEDC_LOW_SPEED_MODE,
        .channel        = LEDC_CHANNEL_0,
        .intr_type      = LEDC_INTR_DISABLE,
        .timer_sel      = LEDC_TIMER_0,
        .duty           = 0, // initial duty cycle set to 0 (stopped)
        .hpoint         = 0
    };
    
    // Apply the channel configuration
    ESP_ERROR_CHECK(ledc_channel_config(&ledc_channel));

   
}

Wheel init_wheel(uint8_t pinEn, uint8_t pinF, uint8_t pinB, ledc_channel_t pwmChannel, ledc_timer_t pwmTimer){
    // Configures forward and backward pins as GPIO outputs
    // Configures LEDC channel for PWM output (ledc_channel_config_t)
    // Returns wheel struct
    // reset pins
    gpio_reset_pin(pinF);
    gpio_reset_pin(pinB);

    // set directions (both direction pins must be outputs)
    gpio_set_direction(pinF, GPIO_MODE_OUTPUT);
    gpio_set_direction(pinB, GPIO_MODE_OUTPUT);

    // set initial levels to low (stopped)
    gpio_set_level(pinF, 0);
    gpio_set_level(pinB, 0);    
    Wheel my_wheel = {
        .pinEn = pinEn,
        .pinF = pinF,
        .pinB = pinB,
        .pwmChannel = pwmChannel
    };
    
    return my_wheel;
}

void drive(Wheel *wheel, uint8_t duty, bool isForward){
    // Set level for GPIO pins
    // Sets and updates duty cycle for PWM channel using ledc_set_duty and ledc_update_duty
    // set the direction pins opposite of each other
    if (isForward) {
        gpio_set_level(wheel->pinF, 1);
        gpio_set_level(wheel->pinB, 0);
    } else {
        gpio_set_level(wheel->pinF, 0);
        gpio_set_level(wheel->pinB, 1);
    }

    // set and update the duty cycle for the PWM channel
    ledc_set_duty(LEDC_LOW_SPEED_MODE, (ledc_channel_t)wheel->pwmChannel, duty);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, (ledc_channel_t)wheel->pwmChannel);
}

void tele_drive(Wheel *wheel, uint8_t input, bool invert){
    // Checks if input is greater than 127, sets direction and duty cycle accordingly
    // Calls drive
    bool isForward;
    uint8_t duty = 0;

    // determine direction and calculate magnitude from center (127)
    if (input > 127) {
        isForward = true;
        // calculate distance from 127 and scale to 0-255 duty range (128 * 2 = 256, cap at 255)
        uint16_t diff = (uint16_t)(input - 127) * 2;
        uint8_t duty;
        if (diff > 255) {
            duty = 255;
        }
        else {
            duty = (uint8_t)diff;
        }
    } else {
        isForward = false;
        uint16_t diff = (uint16_t)(127 - input) * 2;
        uint8_t duty;
        if (diff >255) {
            duty = 255;
        }
        else {
            duty = (uint8_t)diff;
        }
    }

    // apply inverse if requested
    if (invert) {
        isForward = !isForward;
    }

    // call the base drive function
    drive(wheel, duty, isForward);
}