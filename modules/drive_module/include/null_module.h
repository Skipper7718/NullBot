#ifndef __NULL_MODULE_H__
#define __NULL_MODULE_H__

#define NULL_ERR 1;
#define NULL_OK 0;

int init_i2c( int pin_sda, int pin_scl, uint8_t address );
void led_task();
void start_with_callback(void(*callback)(uint8_t *payload, size_t len));

#endif // __NULL_MODULE_H__