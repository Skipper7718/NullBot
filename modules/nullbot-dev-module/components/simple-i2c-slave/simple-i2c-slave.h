#ifndef __SIMPLE_I2C_SLAVE_H__
#define __SIMPLE_I2C_SLAVE_H__

#include <stdbool.h>

#define MAX_DATA_LEN 256
esp_err_t i2c_slave_init();
int i2c_slave_write(uint8_t *buffer, size_t size, int timeout, bool clear_buffer);
void start_i2c_listener();
void i2c_register_listener(void (*handler_function)(uint8_t*, size_t));
typedef void (*callback_func)(uint8_t *payload, size_t len);

#endif