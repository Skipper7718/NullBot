#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdint.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/i2c.h"
#include "esp_err.h"
#include "esp_system.h"
#include "simple_i2c.h"
#include "esp_log.h"
#include "lcd-display.h"

#define I2C_ID I2C_NUM_0
#define FREQ 100000


void serial_read_line(char *buffer, int buffer_len);

void app_main(void)
{
    init_i2c_master();
    init_display();
    fillScreen(TFT_BLACK);

    drawString(10,10,"FULL BUS SCAN", TFT_RED);
    uint8_t addressesi[128];
    int num_found = i2c_scan_bus(addressesi, sizeof(addressesi));
    ESP_LOGI("I²C BUS SCAN", "FOUND %d", num_found);
    fillScreen(TFT_BLACK);
    char text[30];
    for( int i = 0; i < num_found; i++ ) {
        int y = 10;
        sprintf(text, "Found 0x%02x", addressesi[i]);
        drawString(10, y, text, TFT_WHITE);
        y += 30;
    }

    char input[100];
    for( ;; ) {
        serial_read_line(input, sizeof(input));

        if( input[0] == 'w' ) {
            printf("execute write\n");
            uint8_t command[6] = {0x20,1,2,3,4,5};
            int success = i2c_write_bytes(0x31, command, 6, I2C_NUM_0);
            printf("%s\n", success ? "SUCCESS" : "ERROR");
            write_rect(TFT_LIGHTGREY, success ? "SUCCESS WRITE" : "ERROR WRITE");
        }
        else if( input[0] == 'r' ) {
            printf("execute read\n");
            uint8_t byte[5];
            int success = i2c_read_bytes(0x31, byte, 5, I2C_NUM_0);
            success ? printf("Read byte 0x%02x\n", byte[0]) : printf("ERROR\n");
            sprintf(text, "Read 0x%02x", byte[0]);
            write_rect(TFT_GREEN, text);
        }
    }
}

void serial_read_line(char *buffer, int buffer_len) {
    memset(buffer, 0, buffer_len);
    char c = 0;
    int i = 0;

    while( c != '\n' && i < buffer_len-1 ) {
        if( (c = getchar()) != 0xff ) {
            buffer[strlen(buffer)] = c;
            printf("%c", c);
            i++;
        }
        vTaskDelay(50 / portTICK_PERIOD_MS);
    }

}