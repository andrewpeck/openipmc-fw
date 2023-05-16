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

#endif /* APOLLO_I2C_H */
