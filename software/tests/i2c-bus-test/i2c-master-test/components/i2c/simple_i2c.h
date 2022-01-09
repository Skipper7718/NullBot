#define WAIT 500

/**
 * @brief initialize i2c_bus
 */
void init_i2c_master();

/**
 * @brief write bytes to slave
 */
int i2c_write_bytes(uint8_t addr, uint8_t *bytes, size_t bytes_len, int i2c_num);

/**
 * @brief read bytes into buffer from slave
 */
int i2c_read_bytes(uint8_t addr, uint8_t *buffer, size_t read_len, int i2c_num);

/**
 * @brief scan i2c bus, write found addresses in buffer and return number of found addresses
 */
int i2c_scan_bus(uint8_t *found_addresses, size_t buf_size);