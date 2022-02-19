#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "hardware/gpio.h"
#include "pico/bootrom.h"
#include "null_module.h"
#include "pico/multicore.h"

// PORT DEFINITIONS
#define PORT i2c1

// map one range to another
long map(long x, long in_min, long in_max, long out_min, long out_max) {
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

// led blinks in interval if i2c is enabled
void led_task() {
    uint led = 25;
    gpio_init(led);
    gpio_set_dir(led, GPIO_OUT);

    for( ;; ) {
        gpio_put(led, true);
        sleep_ms(500);
        gpio_put(led, false);
        sleep_ms(500);
    }
}

void start_with_callback(void(*callback)(uint8_t *payload, size_t len)) {
    size_t available;
    for( ;; ) { // dont stop listenening for signals
        if( (available = i2c_get_read_available(PORT)) ) {
            uint8_t *buffer = (uint8_t *) malloc(available);
            i2c_read_raw_blocking(PORT, buffer, available);
            callback(buffer, available);
            free(buffer);
        }
    }
}

int init_i2c( int pin_sda, int pin_scl, uint8_t address ) {
    i2c_init(PORT, 100000);
    gpio_set_function(pin_sda, GPIO_FUNC_I2C);
    gpio_set_function(pin_scl, GPIO_FUNC_I2C);
    gpio_pull_up(pin_scl);
    gpio_pull_up(pin_sda);
    i2c_set_slave_mode(PORT, true, address);

    multicore_launch_core1(led_task);
    return NULL_OK;
}

