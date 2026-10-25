#include "bno055.h"

void init_i2c_driver(void){
    // Create bus handle, configure I2C parameters, and apply create new bus using the config and handle
    // Add BNO055 to I2C bus
    i2c_master_bus_config_t i2c_master_config = {
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .i2c_port = I2C_MASTER_NUM,
        .scl_io_num = I2C_MASTER_SCL_IO,
        .sda_io_num = I2C_MASTER_SDA_IO,
        .glitch_ignore_cnt = 7,
        .flags.enable_internal_pullup = true,
    };

    //Create bus handle

    i2c_master_bus_handle_t bus_handle;
    ESP_ERROR_CHECK(i2c_new_master_bus(&i2c_master_config, &bus_handle));

    //configure bno055
    i2c_device_config_t dev_cfg = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7, 
        .device_address = BNO055_ADDR, // variable must be defined, will do inperson.
        .scl_speed_hz = 100000, //I assume 100kH (?)
    };

    ESP_ERROR_CHECK(i2c_master_bus_add_device(bus_handle, &dev_cfg, &i2c_handle));
}

// helper to write a single byte to a register
esp_err_t bno055_write_reg(uint8_t reg, uint8_t data){
    uint8_t data_reg[] = {reg, data};
    return i2c_master_transmit(i2c_handle, data_reg, sizeof(data_reg), -1);
}

// helper to read multiple bytes starting from a register
esp_err_t bno055_read_regs(uint8_t reg, uint8_t *data, size_t len){
    return i2c_master_transmit_receive(i2c_handle, &reg, 1, data, len, -1);
}

void bno055_init(void) {
    init_i2c_driver();
}

void bno055_set_mode(uint8_t mode) {
    bno055_write_reg(BNO055_OPR_MODE_REG, mode);
    vTaskDelay(pdMS_TO_TICKS(20));
}

void bno055_read_euler(int16_t *heading, int16_t *roll, int16_t *pitch) {
    uint8_t buffer[6];
    bno055_read_regs(BNO055_EULER_H_LSB, buffer, 6);
    *heading = (int16_t)((buffer[1] << 8) | buffer[0]);
    *roll    = (int16_t)((buffer[3] << 8) | buffer[2]);
    *pitch   = (int16_t)((buffer[5] << 8) | buffer[4]);
}