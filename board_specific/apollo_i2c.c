#include "apollo_i2c.h"
#include "sense_i2c.h"
#include "tca9546.h"

// addr gets shifted by 1 for 7 bit vs. 8 bit addressing

/*
 * Macros below define transmit and receive functions for the I2C bus connected to I2C3 peripheral.
 * 
 * Each function first writes to the multiplexer (TCA9546A) to pick the correct bus for the transaction.
 * This refers to the SEL() call in the below macros.
 * 
 * The second step is the actual read/write transaction once the correct bus is picked.
 */
#define CREATE_APOLLO_I2C_TX(FNAME, SEL) \
    h7i2c_i2c_ret_code_t FNAME ## _n (uint8_t *data, uint8_t adr, uint16_t bytes) { \
      h7i2c_i2c_ret_code_t stat = 0; \
      stat = SEL (); \
      stat = stat | h7i2c_i2c_clear_error_state_and_write(H7I2C_I2C3, adr<<1, bytes, data, 100); \
      return stat; \
    } \
    h7i2c_i2c_ret_code_t FNAME (uint8_t *data, uint8_t adr) { \
      return FNAME ## _n (data, adr, 1); \
    } 

#define CREATE_APOLLO_I2C_RX(FNAME, SEL) \
    h7i2c_i2c_ret_code_t FNAME ## _n (uint8_t *data, uint8_t adr, uint16_t bytes) { \
      h7i2c_i2c_ret_code_t stat = 0; \
      stat = SEL (); \
      stat = stat | h7i2c_i2c_clear_error_state_and_read(H7I2C_I2C3, adr<<1, bytes, data, 100); \
      return stat; \
    } \
    h7i2c_i2c_ret_code_t FNAME (uint8_t *data, uint8_t adr) { \
      return FNAME ## _n (data, adr, 1); \
    } 

CREATE_APOLLO_I2C_TX (local_i2c_tx, tca9546_sel_local);
CREATE_APOLLO_I2C_TX (cm1_i2c_tx,   tca9546_sel_m1);
CREATE_APOLLO_I2C_TX (cm2_i2c_tx,   tca9546_sel_m2);
CREATE_APOLLO_I2C_TX (zynq_i2c_tx,  tca9546_sel_zynq);

CREATE_APOLLO_I2C_RX (local_i2c_rx, tca9546_sel_local);
CREATE_APOLLO_I2C_RX (cm1_i2c_rx,   tca9546_sel_m1);
CREATE_APOLLO_I2C_RX (cm2_i2c_rx,   tca9546_sel_m2);
CREATE_APOLLO_I2C_RX (zynq_i2c_rx,  tca9546_sel_zynq);

/*
 * Function to check the error state of the I2C peripheral, and clear it if necessary.
 */
h7i2c_i2c_ret_code_t h7i2c_i2c_check_and_clear_error_state(h7i2c_periph_t peripheral) {
  h7i2c_i2c_ret_code_t status = H7I2C_RET_CODE_OK;
  /* Clear the error state if this peripheral is in error state. */
  if (h7i2c_is_in_error(peripheral)) {
    status = h7i2c_clear_error_state(peripheral);
  }

  return status;
}

/*
 * Macro to define I2C read/write functions using the h7i2c-baremetal-driver, 
 * where DRIVER_IO_FUNC refers to the actual read/write function given us by the driver.
 *
 * These read/write functions do the following:
 * 1) Check if the I2C peripheral is in error state, and if necessary, clear it.
 * 2) Do the read/write.
 * 3) Check the peripheral again after the read/write, and if necessary, clear the error state.
 */
#define CREATE_H7I2C_READ_WRITE_FUNC(FNAME, DRIVER_IO_FUNC) \
  h7i2c_i2c_ret_code_t FNAME (h7i2c_periph_t peripheral, uint16_t dev_address, uint16_t data_size, uint8_t *data_buf, uint32_t timeout) { \
    h7i2c_i2c_ret_code_t status = H7I2C_RET_CODE_OK; \
    status = h7i2c_i2c_check_and_clear_error_state(peripheral); \
    if (status != H7I2C_RET_CODE_OK) { \
      return status; \
    } \
    status = DRIVER_IO_FUNC(peripheral, dev_address, data_size, data_buf, timeout); \
    if (status != H7I2C_RET_CODE_OK) { \
      return status; \
    } \
    status = h7i2c_i2c_check_and_clear_error_state(peripheral); \
    return status; \
  }

CREATE_H7I2C_READ_WRITE_FUNC(h7i2c_i2c_clear_error_state_and_read, h7i2c_i2c_read);
CREATE_H7I2C_READ_WRITE_FUNC(h7i2c_i2c_clear_error_state_and_write, h7i2c_i2c_write);