#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "pico/bootrom.h"
#include "hardware/i2c.h"
#include "hardware/gpio.h"
#include "null_module.h"
#include "drive_module.h"

const int num_pins = 8;
const int pins[8] = {
    F_L_FORWARD,
    F_L_REVERSE,
    F_R_FORWARD,
    F_R_REVERSE,
    B_L_FORWARD,
    B_L_REVERSE,
    B_R_FORWARD,
    B_R_REVERSE
};

void init_drive_module() {

    //init pins
    for( int i = 0; i < num_pins; i++ ) {
        gpio_init(pins[i]);
        gpio_set_dir(pins[i], true);
    }

}

void stop_all() {
    for( int i = 0; i < num_pins; i++ ) {
        gpio_put(pins[i], false);
    }
}

void callback(uint8_t *payload, size_t len) {
    uint8_t command = payload[0];
    switch( command ) {
        case 0x01: // stop all motors
            stop_all();
            break;
        case 0x02: // forward
            stop_all();
            gpio_put(F_L_FORWARD, true);
            gpio_put(F_R_FORWARD, true);
            gpio_put(B_L_FORWARD, true);
            gpio_put(B_R_FORWARD, true);
            break;
        case 0x03: // right
            stop_all();
            gpio_put(F_L_FORWARD, true);
            gpio_put(F_R_REVERSE, true);
            gpio_put(B_L_FORWARD, true);
            gpio_put(B_R_REVERSE, true);
            break;
        case 0x04:
            stop_all();
            gpio_put(F_L_REVERSE, true);
            gpio_put(F_R_FORWARD, true);
            gpio_put(B_L_REVERSE, true);
            gpio_put(B_R_FORWARD, true);
            break;
    }
}

int main() {
    init_i2c(26, 27, 0x56);
    sleep_ms(1000);
    init_drive_module();

    // self test
    // uint8_t payload = 0x02;
    // callback(&payload, 1);
    // sleep_ms(1000);
    // payload = 0x03;
    // callback(&payload, 1);
    // sleep_ms(1000);
    // payload = 0x04;
    // callback(&payload, 1);
    // sleep_ms(1000);
    // payload = 0x01;
    // callback(&payload, 1);
    start_with_callback(callback);
    for( ;; ) { tight_loop_contents(); }


    return 0;
}
