#pragma once

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/i2c_master.h"
#include "esp_log.h"

#define I2C_MASTER_NUM       I2C_NUM_0
#define I2C_MASTER_SDA_IO    GPIO_NUM_37
#define I2C_MASTER_SCL_IO    GPIO_NUM_38
#define BNO055_ADDR          0x28 // Define

// Essential BNO055 Register Maps
#define BNO055_OPR_MODE_REG  0x3D // Define
#define BNO055_EULER_H_LSB   0x1A // Define

// Operating Modes
#define OPR_MODE_CONFIG      0x00
#define OPR_MODE_NDOF        0x0C

static i2c_master_dev_handle_t i2c_handle;

void init_i2c_driver(void);
esp_err_t bno055_write_reg(uint8_t reg, uint8_t data);
esp_err_t bno055_read_regs(uint8_t reg, uint8_t *data, size_t len);
void bno055_init(void);
void bno055_set_mode(uint8_t mode);
void bno055_read_euler(int16_t *heading, int16_t *roll, int16_t *pitch);