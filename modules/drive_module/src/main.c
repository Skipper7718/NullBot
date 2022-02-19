#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include "pico/bootrom.h"
#include "hardware/i2c.h"
#include "null_module.h"
#include "drive_module.h"

const int num_pins = 8;
static int level = 255;

long remap( long x ) {
    return map(x, 0, 255, 0, 65535);
}

void gpio_put_analog( int gpio, bool value ) {
    pwm_set_gpio_level(gpio, value ? remap(level) : 0);
}

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
    pwm_config config = pwm_get_default_config();
    for( int i = 0; i < num_pins; i++ ) {
        gpio_set_function(pins[i], GPIO_FUNC_PWM);
        int slice_num = pwm_gpio_to_slice_num(pins[i]);
        pwm_init(slice_num, &config, true);
    }
}

void stop_all() {
    for( int i = 0; i < num_pins; i++ ) {
        gpio_put_analog(pins[i], false);
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
            gpio_put_analog(F_L_FORWARD, true);
            gpio_put_analog(F_R_FORWARD, true);
            gpio_put_analog(B_L_FORWARD, true);
            gpio_put_analog(B_R_FORWARD, true);
            break;
        case 0x03: // right
            stop_all();
            gpio_put_analog(F_L_FORWARD, true);
            gpio_put_analog(F_R_REVERSE, true);
            gpio_put_analog(B_L_FORWARD, true);
            gpio_put_analog(B_R_REVERSE, true);
            break;
        case 0x04: // left
            stop_all();
            gpio_put_analog(F_L_REVERSE, true);
            gpio_put_analog(F_R_FORWARD, true);
            gpio_put_analog(B_L_REVERSE, true);
            gpio_put_analog(B_R_FORWARD, true);
            break;
        case 0x05: // backward
            stop_all();
            gpio_put_analog(F_L_REVERSE, true);
            gpio_put_analog(F_R_REVERSE, true);
            gpio_put_analog(B_L_REVERSE, true);
            gpio_put_analog(B_R_REVERSE, true);
            break;
        case 0x06: // simple right
            stop_all();
            gpio_put_analog(F_L_FORWARD, true);
            gpio_put_analog(B_L_FORWARD, true);
            break;
        case 0x07: // simple left
            stop_all();
            gpio_put_analog(F_R_FORWARD, true);
            gpio_put_analog(B_R_FORWARD, true);
            break;
        case 0x08: // set level
            if( len < 2 ) break;
            level = payload[1];
            break;
    }
}

int main() {
    init_i2c(26, 27, 0x56);
    sleep_ms(1000);
    init_drive_module();

    // self test
    // for( uint8_t payload = 0x07; payload > 0; payload-- ) {
    //     callback(&payload, 1);
    //     sleep_ms(1000);
    // }
    // uint8_t settings[2] = {0x08, 0xc0};
    // callback(settings, sizeof(settings));
    // for( uint8_t payload = 0x07; payload > 0; payload-- ) {
    //     callback(&payload, 1);
    //     sleep_ms(1000);
    // }

    // module loop
    start_with_callback(callback);
    for( ;; ) { tight_loop_contents(); }


    return 0;
}
