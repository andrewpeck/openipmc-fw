#ifndef APOLLO_I2C_H
#define APOLLO_I2C_H

#include "sense_i2c.h"
#include "h7i2c_bare.h"

// single byte flavors of i2c tx/rx
h7i2c_i2c_ret_code_t local_i2c_tx (uint8_t *data, uint8_t adr);
h7i2c_i2c_ret_code_t local_i2c_rx (uint8_t *data, uint8_t adr);
h7i2c_i2c_ret_code_t cm1_i2c_tx (uint8_t *data, uint8_t adr);
h7i2c_i2c_ret_code_t cm1_i2c_rx (uint8_t *data, uint8_t adr);
h7i2c_i2c_ret_code_t cm2_i2c_tx (uint8_t *data, uint8_t adr);
h7i2c_i2c_ret_code_t cm2_i2c_rx (uint8_t *data, uint8_t adr);
h7i2c_i2c_ret_code_t zynq_i2c_tx (uint8_t *data, uint8_t adr);
h7i2c_i2c_ret_code_t zynq_i2c_rx (uint8_t *data, uint8_t adr);

// multi-byte flavors of i2c tx/rx
h7i2c_i2c_ret_code_t local_i2c_tx_n (uint8_t *data, uint8_t adr, uint16_t bytes);
h7i2c_i2c_ret_code_t local_i2c_rx_n (uint8_t *data, uint8_t adr, uint16_t bytes);
h7i2c_i2c_ret_code_t cm1_i2c_tx_n (uint8_t *data, uint8_t adr, uint16_t bytes);
h7i2c_i2c_ret_code_t cm1_i2c_rx_n (uint8_t *data, uint8_t adr, uint16_t bytes);
h7i2c_i2c_ret_code_t cm2_i2c_tx_n (uint8_t *data, uint8_t adr, uint16_t bytes);
h7i2c_i2c_ret_code_t cm2_i2c_rx_n (uint8_t *data, uint8_t adr, uint16_t bytes);
h7i2c_i2c_ret_code_t zynq_i2c_tx_n (uint8_t *data, uint8_t adr, uint16_t bytes);
h7i2c_i2c_ret_code_t zynq_i2c_rx_n (uint8_t *data, uint8_t adr, uint16_t bytes);

/* 
 * Wrapper for I2C reads and writes with h7i2c-baremetal-driver. 
 * These functions first check if the peripheral is in an error state, and if so they clear the error state.
 * After that, they perform the read/write.
 */

h7i2c_i2c_ret_code_t h7i2c_i2c_clear_error_state_and_read(h7i2c_periph_t peripheral, uint16_t dev_address, uint16_t data_size, uint8_t *data_buf, uint32_t timeout);
h7i2c_i2c_ret_code_t h7i2c_i2c_clear_error_state_and_write(h7i2c_periph_t peripheral, uint16_t dev_address, uint16_t data_size, uint8_t *data_buf, uint32_t timeout);


#endif /* APOLLO_I2C_H */
