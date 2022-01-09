#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "driver/i2c.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "simple-i2c-slave.h"

void (*i2c_receive_handler)(uint8_t*, size_t);

esp_err_t i2c_slave_init()
{
    int i2c_slave_port = CONFIG_I2C_PORT;
    i2c_config_t conf_slave = {
        .sda_io_num = CONFIG_PIN_SDA,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_io_num = CONFIG_PIN_SCL,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .mode = I2C_MODE_SLAVE,
        .slave.addr_10bit_en = 0,
        .slave.slave_addr = CONFIG_SLAVE_ADDRESS,
    };
    esp_err_t err = i2c_param_config(i2c_slave_port, &conf_slave);
    if (err != ESP_OK) {
        return err;
    }
    return i2c_driver_install(i2c_slave_port, conf_slave.mode, (MAX_DATA_LEN * 2), (MAX_DATA_LEN * 2), 0);
    i2c_reset_rx_fifo(i2c_slave_port);
    i2c_reset_tx_fifo(i2c_slave_port);
}

int i2c_slave_write(uint8_t *buffer, size_t size, int timeout, bool clear_buffer) {
    if( clear_buffer )
        i2c_reset_tx_fifo(CONFIG_I2C_PORT);
    return i2c_slave_write_buffer(CONFIG_I2C_PORT, buffer, size, timeout);
}

void start_i2c_listener() {
    uint8_t buffer[MAX_DATA_LEN];
    memset(buffer, 0, MAX_DATA_LEN);
    for( ;; ) {
        size_t num_bytes = i2c_slave_read_buffer(CONFIG_I2C_PORT, buffer, MAX_DATA_LEN, CONFIG_I2C_READ_TIMEOUT/portTICK_PERIOD_MS);
        if( num_bytes > 0 ) {
            i2c_receive_handler(buffer, num_bytes);
            memset(buffer, 0, MAX_DATA_LEN);
        }
    }
}

void i2c_register_listener(void (*handler_function)(uint8_t*, size_t)) {
    i2c_receive_handler = handler_function;
}