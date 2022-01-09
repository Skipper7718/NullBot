#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdint.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/i2c.h"
#include "esp_err.h"
#include "simple_i2c.h"
#include "esp_log.h"

void init_i2c_master() {
    int i2c_num = CONFIG_I2C_PORT;
    i2c_config_t config = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = CONFIG_PIN_SDA,
        .scl_io_num = CONFIG_PIN_SCL,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = CONFIG_CLOCK_SPEED
    };

    i2c_param_config(i2c_num, &config);
    i2c_driver_install(i2c_num, I2C_MODE_MASTER, 0, 0, true);
}

int i2c_write_bytes(uint8_t addr, uint8_t *bytes, size_t bytes_len, int i2c_num) {
    // send start byte
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (addr << 1) | I2C_MASTER_WRITE, I2C_MASTER_ACK);

    // send payload and end transmission
    for (int i = 0; i < bytes_len; i++) {
        i2c_master_write_byte(cmd, bytes[i], I2C_MASTER_ACK);
    }
    i2c_master_stop(cmd);

    // send and check for errors
    esp_err_t err = i2c_master_cmd_begin(i2c_num, cmd, WAIT / portTICK_PERIOD_MS);
    
    // delete command and free memory
    i2c_cmd_link_delete(cmd);

    if (err == ESP_OK)
        return 1;
    else
        return 0;

}

int i2c_read_bytes(uint8_t addr, uint8_t *buffer, size_t read_len, int i2c_num) {
    // send start byte
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (addr << 1) | I2C_MASTER_READ, I2C_MASTER_ACK);

    // read bytes to buffer
    for (int i = 0; i < read_len - 1; i++) {
        i2c_master_read_byte(cmd, &buffer[i], I2C_MASTER_ACK);
    }
    i2c_master_read_byte(cmd, &buffer[read_len - 1], I2C_MASTER_NACK);

    // stop transmission ans check for errors
    i2c_master_stop(cmd);
    esp_err_t err = i2c_master_cmd_begin(i2c_num, cmd, WAIT / portTICK_PERIOD_MS);

    //delete command and free memory
    i2c_cmd_link_delete(cmd);
    
    if (err == ESP_OK)
        return 1;
    else
        return 0;
}

int i2c_scan_bus(uint8_t *found_addresses, size_t buf_size) {
    if( buf_size < 128 )
        return 0;

    uint8_t address;
    int index = 0;
    for (int i = 0; i < 128; i += 16) {
        for (int j = 0; j < 16; j++) {
            address = i + j;
            i2c_cmd_handle_t cmd = i2c_cmd_link_create();
            i2c_master_start(cmd);
            i2c_master_write_byte(cmd, (address << 1) | I2C_MASTER_WRITE, I2C_MASTER_NACK);
            i2c_master_stop(cmd);
            esp_err_t ret = i2c_master_cmd_begin(CONFIG_I2C_PORT, cmd, 10 / portTICK_RATE_MS);
            i2c_cmd_link_delete(cmd);
            if (ret == ESP_OK) {
                found_addresses[index++] = address;
            }
        }
    }
    return index;
}