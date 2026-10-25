// Included header files
#include <stdio.h>
#include <inttypes.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_chip_info.h"
#include "esp_flash.h"
#include "esp_system.h"
#include "driver/uart.h"     // Native ESP-IDF UART driver
#include "driver/ledc.h"     // Native ESP-IDF LEDC (PWM) driver
#include "ultrasonic.h"
#include "wheel.h"
#include "freertos/queue.h"
#include "bno055.h"

// Defines for GPIO pins
#define LEFT_EN       GPIO_NUM_16
#define LEFT_FORWARD  GPIO_NUM_17
#define LEFT_BACKWARD GPIO_NUM_18

#define RIGHT_EN       GPIO_NUM_4
#define RIGHT_FORWARD  GPIO_NUM_5
#define RIGHT_BACKWARD GPIO_NUM_6

#define L_US_TRIG GPIO_NUM_9
#define L_US_ECHO GPIO_NUM_10

#define F_US_TRIG GPIO_NUM_11
#define F_US_ECHO GPIO_NUM_12

#define R_US_TRIG GPIO_NUM_13
#define R_US_ECHO GPIO_NUM_14

// Global Variables and Predefined Types
QueueHandle_t cmd_queue;

Wheel left;
Wheel right;

Ultrasonic leftSense;
Ultrasonic frontSense;
Ultrasonic rightSense;
Ultrasonic *ultrasonics[3] = { &leftSense, &frontSense, &rightSense };
double ultrasonicFeedback[3];
double heading;

typedef struct {
    uint8_t left;
    uint8_t right;
} Command;


// Tasks
void uart_rx_task(void *pvParameters)
{
    uint8_t data[3];
    Command cmd;
    // Write code to read wheel commands from UART
    while(1){
        // read 3 bytes from UART
        int len = uart_read_bytes(UART_NUM_1, data, 3, portMAX_DELAY);
        if (len == 3) {
            // byte 1 is ignored, byte 2 = left wheel, byte 3 = right wheel
            cmd.left = data[1];
            cmd.right = data[2];
            
            // send command packet to queue
            xQueueSend(cmd_queue, &cmd, 0);
        }
    }
}

void uart_tx_task(void *pvParameters){
    // Write code to send ultrasonic sensor and IMU over UART
    while(1){
        // buffer or transmission format matching requirement (ex: word "mars", 3 bytes for US feedback, 1 byte for heading)
        char header[] = "mars";
        uint8_t us_bytes[3];
        
        // scale or cast ultrasonic feedback into bytes for transmission
        for(int i = 0; i < 3; i++) {
            us_bytes[i] = (uint8_t)(ultrasonicFeedback[i] > 255 ? 255 : ultrasonicFeedback[i]);
        }
        uint8_t heading_byte = (uint8_t)((int)heading % 256);

        // send over UART
        uart_write_bytes(UART_NUM_1, header, 4);
        uart_write_bytes(UART_NUM_1, (char *)us_bytes, 3);
        uart_write_bytes(UART_NUM_1, (char *)&heading_byte, 1);

        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

void ultrasonic_task(void *pvParameters){
    // Write code to record ultrasonic sensor feedback in array of doubles
    int index = (int)pvParameters;
    while(1){
        // In this loop,  Implement a vTaskDelay of 50ms
        ultrasonicFeedback[index] = calculate_distance(ultrasonics[index]);
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}

void imu_task(void *pvParameters){
    // Write code to initialize IMU and call vTaskDelay for 30ms
    // Set operational mode to NDOF (Absolute Orientation Fusion) and call vTaskDelay for 30ms
    // Create a data buffer 

    // Initialize IMU
    bno055_init();
    vTaskDelay(pdMS_TO_TICKS(30));
    
    // Set operational mode to NDOF (Absolute Orientation Fusion)
    bno055_set_mode(OPR_MODE_NDOF);
    vTaskDelay(pdMS_TO_TICKS(30));
    
    int16_t heading_val, roll_val, pitch_val;

    while(1){
        // Read euler data from BNO055IMU registers
        // Store heading, roll, and pitch values in degrees in int16_t variables
        // Implement a vTaskDelay for 100ms
        bno055_read_euler(&heading_val, &roll_val, &pitch_val);
        heading = (double)heading_val / 16.0; 
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

void control_task(void *pvParameters){
    Command cmd;
    while(1){
        // Write code to use the proper commands for driving the left and right wheels
        if (xQueueReceive(cmd_queue, &cmd, portMAX_DELAY) == pdTRUE) {
            // Drive calls must go inside the block so 'cmd' is guaranteed to be initialized
            tele_drive(&left, cmd.left, false);
            tele_drive(&right, cmd.right, false);
        }
    }
}

void app_main(void)
{
    // Initialize peripherals (UART, PWM, I2C, Ultrasonic sensors, and wheels)
    // initialize PWM Timer & Wheels
    init_hardware_pwm_timer();
    left = init_wheel(LEFT_EN, LEFT_FORWARD, LEFT_BACKWARD, LEDC_CHANNEL_0, LEDC_TIMER_0);
    right = init_wheel(RIGHT_EN, RIGHT_FORWARD, RIGHT_BACKWARD, LEDC_CHANNEL_1, LEDC_TIMER_0);

    // initialize Ultrasonic Sensors
    leftSense = init_ultrasonic(L_US_TRIG, L_US_ECHO);
    frontSense = init_ultrasonic(F_US_TRIG, F_US_ECHO);
    rightSense = init_ultrasonic(R_US_TRIG, R_US_ECHO);

    // create command queue (e.g., holds 10 Command items)
    cmd_queue = xQueueCreate(10, sizeof(Command));

    // Create tasks
    if (cmd_queue != NULL)
    {
        xTaskCreate(control_task, "control_task", 2048, NULL, 5, NULL);
        xTaskCreate(uart_rx_task, "uart_rx_task", 2048, NULL, 5, NULL);
        xTaskCreate(uart_tx_task, "uart_tx_task", 2048, NULL, 5, NULL);
        xTaskCreate(imu_task, "imu_task", 2048, NULL, 5, NULL);
        for (size_t i = 0; i < sizeof(ultrasonics) / sizeof(ultrasonics[0]); i++)
            xTaskCreate(ultrasonic_task, "ultrasonic_task", 2048, (void *) i, 5, NULL);
    }
}