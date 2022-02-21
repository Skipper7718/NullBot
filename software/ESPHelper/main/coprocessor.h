#ifndef __COPROCESSOR_H__
#define __COPROCESSOR_H__

int serial_read_command(char *buffer, int buffer_len);
void print_display(const char *message);
void interpret_signal(char *input, int signal_length);
esp_err_t fill_led(uint32_t r, uint32_t g, uint32_t b);
esp_err_t print_led(uint32_t led_num, uint32_t r, uint32_t g, uint32_t b);

enum INSTRUCTION_SET {
    i_stop = 0xfe,
    i_led_set = 0x10,
    i_led_fill = 0x11,
    i_read_scan = 0x20,
    i_write_address = 0x21,
    i_read_address = 0x22,
    i_display_error = 0x30,
    i_display_thread_failure = 0x31,
    i_display_all_running = 0x32,
};

#define RMT_TX_CHANNEL RMT_CHANNEL_0
#define RMT_TX_GPIO 27
#define LED_LEN 14

#endif