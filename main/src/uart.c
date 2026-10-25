#include "uart.h"

int UART_NUM = 0;

void init_uart_driver(void) {
    QueueHandle_t uart_queue;
    
    uart_config_t uart_config = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_CTS_RTS,
        .rx_flow_ctrl_thresh = 122,
    };
    
    // Configure parameters, set pins, and install driver in proper order
    ESP_ERROR_CHECK(uart_param_config(UART_NUM, &uart_config));
    ESP_ERROR_CHECK(uart_set_pin(UART_NUM, S3_TX_PIN, S3_RX_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));
    ESP_ERROR_CHECK(uart_driver_install(ROBOT_UART_PORT, UART_BUF_SIZE, UART_BUF_SIZE, 10, &uart_queue, 0));
}

bool uart_read(uint8_t *data, uint8_t size) {
    // Read serial bytes with 20ms delay 
    int bytes_read = uart_read_bytes(UART_NUM, data, size, pdMS_TO_TICKS(20));
    
    // Return true if expected bytes match and first byte is 0xFF
    if (bytes_read == size && data[0] == 0xFF) {
        return true;
    }
    return false;
}

void uart_write(void *packet, size_t size) {
    // Write serial bytes
    uart_write_bytes(UART_NUM, packet, size);
}