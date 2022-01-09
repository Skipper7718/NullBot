#include <stdio.h>
#include <stdbool.h>
#include "driver/i2c.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "simple-i2c-slave.h"
#include "esp_log.h"
#include "esp_err.h"

void handler(uint8_t *buffer, size_t len);

void app_main(void)
{
    esp_err_t ret = i2c_slave_init();
    if( ret != ESP_OK )
        printf("ERROR INIT\n");

    i2c_register_listener(handler);
    xTaskCreate(start_i2c_listener, "I2C input listener", 2048, NULL, 3, NULL);

}

void handler(uint8_t *buffer, size_t len) {
    printf("\n------------------\ntrigger: got [%d] bytes:\n", len);
    for( int i = 0; i < len; i++ ) {
        printf("0x%02x ", buffer[i]);
        fflush(stdout);
    }
    putchar('\n');

    uint8_t response[1] = {0x44};
    switch( buffer[0] ) {
        case 0x10:
            i2c_slave_write(response, 1, 100, true);
            ESP_LOGI("INFO", "ECHO CALL");
            break;
        case 0x20:
            response[0] = 0x65;
            i2c_slave_write(response, 1, 100, true);
            ESP_LOGI("INFO", "NEW ECHO CALL");
            break;
        default:
            ESP_LOGI("WARNING:", "unknown command");
            break;
    }
}