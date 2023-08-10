/**
 * apollo_i2c.c and apollo_i2c.h implement the I2C transactions on the sensor bus (I2C3) and the
 * management bus (I2C4), using the underlying API calls from the h7i2c-baremetal-driver.
 * 
 * A typical I2C transaction over the sensor bus on Apollo will do the following:
 * - Write to the multiplexer to select which bus (SM, CM1, CM2, Zynq) the transaction is going to occur.
 * - Do the actual read/write transaction.
 * 
 * Both single byte and multi byte versions of reads (Rx) and writes (Tx) are defined within this file.
 */

#ifndef APOLLO_I2C_H
#define APOLLO_I2C_H

#include "h7i2c_bare.h"

/* Enum representing the bus we are picking for a transaction over the sensor bus. */
typedef enum {
    SENSOR_BUS_LOCAL,
    SENSOR_BUS_CM1,
    SENSOR_BUS_CM2,
    SENSOR_BUS_ZYNQ
} i2c3_bus_type_t;

/* Enum representing if the I2C transaction is a read or write. */
typedef enum {
    APOLLO_I2C_BUS_READ_TRANSACTION,
    APOLLO_I2C_BUS_WRITE_TRANSACTION
} i2c3_transaction_type_t;

/* Single byte versions of the read/write functions over the sensor bus. */
h7i2c_i2c_ret_code_t local_i2c_tx (uint8_t *data, uint8_t adr);
h7i2c_i2c_ret_code_t local_i2c_rx (uint8_t *data, uint8_t adr);
h7i2c_i2c_ret_code_t cm1_i2c_tx (uint8_t *data, uint8_t adr);
h7i2c_i2c_ret_code_t cm1_i2c_rx (uint8_t *data, uint8_t adr);
h7i2c_i2c_ret_code_t cm2_i2c_tx (uint8_t *data, uint8_t adr);
h7i2c_i2c_ret_code_t cm2_i2c_rx (uint8_t *data, uint8_t adr);
h7i2c_i2c_ret_code_t zynq_i2c_tx (uint8_t *data, uint8_t adr);
h7i2c_i2c_ret_code_t zynq_i2c_rx (uint8_t *data, uint8_t adr);

/* Multi byte versions of the read/write functions over the sensor bus. */
h7i2c_i2c_ret_code_t local_i2c_tx_n (uint8_t *data, uint8_t adr, uint16_t bytes);
h7i2c_i2c_ret_code_t local_i2c_rx_n (uint8_t *data, uint8_t adr, uint16_t bytes);
h7i2c_i2c_ret_code_t cm1_i2c_tx_n (uint8_t *data, uint8_t adr, uint16_t bytes);
h7i2c_i2c_ret_code_t cm1_i2c_rx_n (uint8_t *data, uint8_t adr, uint16_t bytes);
h7i2c_i2c_ret_code_t cm2_i2c_tx_n (uint8_t *data, uint8_t adr, uint16_t bytes);
h7i2c_i2c_ret_code_t cm2_i2c_rx_n (uint8_t *data, uint8_t adr, uint16_t bytes);
h7i2c_i2c_ret_code_t zynq_i2c_tx_n (uint8_t *data, uint8_t adr, uint16_t bytes);
h7i2c_i2c_ret_code_t zynq_i2c_rx_n (uint8_t *data, uint8_t adr, uint16_t bytes);

/* I2C write-then-read functions over CM1 bus with a repeated start condition. */
h7i2c_i2c_ret_code_t cm1_i2c_tx_and_rx(uint8_t *wr_data, uint8_t *rd_data, uint8_t adr);
h7i2c_i2c_ret_code_t cm1_i2c_tx_and_rx_n(uint8_t *wr_data, uint8_t *rd_data, uint16_t wr_bytes, uint16_t rd_bytes, uint8_t adr);

/* 
 * Wrapper for I2C reads and writes with h7i2c-baremetal-driver. 
 * These functions first check if the peripheral is in an error state, and if so they clear the error state.
 * After that, they perform the read/write, and finally clear the error state again (if need be).
 */

h7i2c_i2c_ret_code_t h7i2c_i2c_check_and_clear_error_state(h7i2c_periph_t peripheral);

h7i2c_i2c_ret_code_t h7i2c_i2c_clear_error_state_and_read(h7i2c_periph_t peripheral, uint16_t dev_address, uint16_t data_size, uint8_t *data_buf, uint32_t timeout);
h7i2c_i2c_ret_code_t h7i2c_i2c_clear_error_state_and_write(h7i2c_periph_t peripheral, uint16_t dev_address, uint16_t data_size, uint8_t *data_buf, uint32_t timeout);
h7i2c_i2c_ret_code_t h7i2c_i2c_clear_error_state_and_write_then_read(h7i2c_periph_t peripheral, uint16_t dev_address, uint16_t wr_data_size, uint16_t rd_data_size, uint8_t *wr_data_buf, uint8_t *rd_data_buf, uint32_t timeout);

#endif /* APOLLO_I2C_H */
