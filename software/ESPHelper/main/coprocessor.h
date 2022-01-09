#ifndef __COPROCESSOR_H__
#define __COPROCESSOR_H__

int serial_read_command(char *buffer, int buffer_len);
void print_display(const char *message);
void interpret_signal(char *input, int signal_length);

enum INSTRUCTION_SET {
    i_stop = 0xfe,
    i_read_scan = 0x20,
    i_write_address = 0x21,
    i_read_address = 0x22,
    i_display_error = 0x30,
    i_display_thread_failure = 0x31,
    i_display_all_running = 0x32,
    i_movement_mask = 0x40
};

#endif