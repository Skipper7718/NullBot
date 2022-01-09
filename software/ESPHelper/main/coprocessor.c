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
#include "sh1106.h"
#include "coprocessor.h"

int num_found_addresses = 0;
uint8_t found_addresses[128];

void app_main(void)
{
    //INITIALIZE COMPONENTS
    init_i2c_master();
    sh1106_init();

    //SCAN FOR ADDRESSES
    print_display("\nFULL BUS SCAN");
    vTaskDelay(500 / portTICK_PERIOD_MS);
    print_display("\nFULL BUS SCAN\nrunning...");

    num_found_addresses = i2c_scan_bus(found_addresses, sizeof(found_addresses));

    print_display("\nFULL BUS SCAN\nrunning...\nDONE!");
    vTaskDelay(500 / portTICK_PERIOD_MS);
    sh1106_display_clear(NULL);
    char text[32];
    char output[128];
    memset(output, 0, sizeof(output));
    if( num_found_addresses ) {
        for( int i = 0; i < num_found_addresses; i++ ) {
            sprintf(text, "Found 0x%02x\n", found_addresses[i]);
            strcat(output, text);
        }
        print_display(output);
    } else {
        print_display("\nERROR\nNO DEVICES FOUND");
    }

    char input[100];

    //main loop for interpreting signals
    for( ;; ) {
        int signal_length = serial_read_command(input, sizeof(input));
        interpret_signal(input, signal_length);
    }
}

void interpret_signal(char *input, int signal_length) {
    uint8_t payload[100]; //MAX TRANSFER SIZE 100 bytes
    uint8_t address;
    char text[100];
    memset(payload, 0, sizeof(payload));

    switch( input[0] ) {
        case i_read_scan:
            sprintf(text, "ECHO %d ADDRESSES\nADDRESS 0: 0x%02x | %d",num_found_addresses, found_addresses[0], found_addresses[0]);
            print_display(text);
            putchar((unsigned char) num_found_addresses);
            for(int i = 0; i < num_found_addresses; i++) {
                putchar((unsigned char) found_addresses[i]);
            }
            putchar(i_stop);
            break;

        case i_write_address:
            print_display("----------\nWRITE ADDRESS\n----------");
            address = input[1];
            for( int i = 2; i < signal_length; i++ ) {
                payload[i-2] = (uint8_t) input[i];
            }
            i2c_write_bytes(address, payload, (signal_length - 2), CONFIG_I2C_PORT);
            break;
        
        case i_read_address:
            print_display("----------\nREAD ADDRESS\n----------");
            uint8_t byte = 0;
            address = input[1];
            size_t num_bytes = input[2];
            i2c_read_bytes(address, payload, num_bytes, CONFIG_I2C_PORT);
            for( int i = 0; i < num_bytes; i++ ) {
                byte = payload[i];
                if( byte != 0 || byte != 0xff ) {
                    putchar(byte);
                }
            }
            putchar(i_stop);
            break;

        case i_display_error:
            print_display("\n----- !!!! -----\n\nERROR\n\n----- !!!! -----");
            break;
        
        case i_display_thread_failure:
            print_display("\n----- !!!! -----\n\nTHREAD FAILED\n\n----- !!!! -----");
            break;

        case i_display_all_running:
            print_display("\n----- !!!! -----\n\nALL RUNNING\n\n----- !!!! -----");
            break;
        default:
            print_display("\n----- !!!! -----\n\nunrecognized\n\n----- !!!! -----");
            break;
    }
}

int serial_read_command(char *buffer, int buffer_len) {
    memset(buffer, 0, buffer_len);
    char c = 0;
    int i = 0;

    while( c != i_stop && i < buffer_len-1 ) {
        if( (c = getchar()) != 0xff ) {
            buffer[strlen(buffer)] = c;
            i++;
        }
        vTaskDelay(50 / portTICK_PERIOD_MS);
    }
    return i;

}

void print_display(const char *message) {
    sh1106_display_clear(NULL);
    sh1106_display_text(message);
}