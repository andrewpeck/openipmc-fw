#include "apollo_i2c.h"
#include "apollo_i2c_mutex.h"
#include "tca9546.h"

/*
 * Underlying function to call for an I2C read/write transaction over the I2C3 bus.
 */
static h7i2c_i2c_ret_code_t apollo_do_i2c3_transaction(uint8_t* data, uint8_t adr, uint16_t bytes, i2c3_bus_type_t i2c3_bus, i2c3_transaction_type_t transaction_type) {
  /* Take the mutex for I2C3 bus. If we fail to get the mutex, return BUSY status code. */
  if (apollo_i2c3_mutex_lock(100) != 0) { return H7I2C_RET_CODE_BUSY; }

  /* Write to the mux to pick the correct I2C bus. */
  h7i2c_i2c_ret_code_t status_mux = H7I2C_RET_CODE_OK;

  switch (i2c3_bus)
  {
  case I2C3_BUS_LOCAL:
    status_mux = tca9546_sel_local();
    break;
  case I2C3_BUS_CM1:
    status_mux = tca9546_sel_m1();
    break;
  case I2C3_BUS_CM2:
    status_mux = tca9546_sel_m2();
    break;
  case I2C3_BUS_ZYNQ:
    status_mux = tca9546_sel_zynq();
    break;
  default:
    return H7I2C_RET_CODE_INVALID_ARGS;
  }

  /* Check if the write to the mux went through. */
  if (status_mux != H7I2C_RET_CODE_OK) {
    return status_mux;
  }

  /* Do the actual write or read. */
  h7i2c_i2c_ret_code_t status_io = H7I2C_RET_CODE_OK;
  if (transaction_type == I2C3_WRITE_TRANSACTION) {
    status_io = h7i2c_i2c_clear_error_state_and_write(H7I2C_I2C3, adr<<1, bytes, data, 100);
  }
  else if (transaction_type == I2C3_READ_TRANSACTION) {
    status_io = h7i2c_i2c_clear_error_state_and_read(H7I2C_I2C3, adr<<1, bytes, data, 100);
  }
  else {
    return H7I2C_RET_CODE_INVALID_ARGS;
  }

  /* Release the mutex for the I2C3 bus and return. */
  apollo_i2c3_mutex_release();
  return status_io;
}

/*
 * The reads/write functions (over the I2C3 bus) for the use of rest of the software are
 * defined below. Each read/write function has a multi-byte and a single-byte flavor.
 */

/* Reads over the local I2C bus. */
h7i2c_i2c_ret_code_t local_i2c_rx_n(uint8_t *data, uint8_t adr, uint16_t bytes) {
  return apollo_do_i2c3_transaction(data, adr, bytes, I2C3_BUS_LOCAL, I2C3_READ_TRANSACTION);
}
h7i2c_i2c_ret_code_t local_i2c_rx(uint8_t *data, uint8_t adr) {
  return local_i2c_rx_n(data, adr, 1);
}

/* Writes over the local I2C bus. */
h7i2c_i2c_ret_code_t local_i2c_tx_n(uint8_t *data, uint8_t adr, uint16_t bytes) {
  return apollo_do_i2c3_transaction(data, adr, bytes, I2C3_BUS_LOCAL, I2C3_WRITE_TRANSACTION);
}
h7i2c_i2c_ret_code_t local_i2c_tx(uint8_t *data, uint8_t adr) {
  return local_i2c_tx_n(data, adr, 1);
}

/* Reads over the CM1 bus. */
h7i2c_i2c_ret_code_t cm1_i2c_rx_n(uint8_t *data, uint8_t adr, uint16_t bytes) {
  return apollo_do_i2c3_transaction(data, adr, bytes, I2C3_BUS_CM1, I2C3_READ_TRANSACTION);
}
h7i2c_i2c_ret_code_t cm1_i2c_rx(uint8_t *data, uint8_t adr) {
  return cm1_i2c_rx_n(data, adr, 1);
}

/* Writes over the CM1 bus. */
h7i2c_i2c_ret_code_t cm1_i2c_tx_n(uint8_t *data, uint8_t adr, uint16_t bytes) {
  return apollo_do_i2c3_transaction(data, adr, bytes, I2C3_BUS_CM1, I2C3_WRITE_TRANSACTION);
}
h7i2c_i2c_ret_code_t cm1_i2c_tx(uint8_t *data, uint8_t adr) {
  return cm1_i2c_tx_n(data, adr, 1);
}

/* Reads over the CM2 bus. */
h7i2c_i2c_ret_code_t cm2_i2c_rx_n(uint8_t *data, uint8_t adr, uint16_t bytes) {
  return apollo_do_i2c3_transaction(data, adr, bytes, I2C3_BUS_CM2, I2C3_READ_TRANSACTION);
}
h7i2c_i2c_ret_code_t cm2_i2c_rx(uint8_t *data, uint8_t adr) {
  return cm2_i2c_rx_n(data, adr, 1);
}

/* Writes over the CM1 bus. */
h7i2c_i2c_ret_code_t cm2_i2c_tx_n(uint8_t *data, uint8_t adr, uint16_t bytes) {
  return apollo_do_i2c3_transaction(data, adr, bytes, I2C3_BUS_CM2, I2C3_WRITE_TRANSACTION);
}
h7i2c_i2c_ret_code_t cm2_i2c_tx(uint8_t *data, uint8_t adr) {
  return cm2_i2c_tx_n(data, adr, 1);
}

/* Reads over the Zynq bus. */
h7i2c_i2c_ret_code_t zynq_i2c_rx_n(uint8_t *data, uint8_t adr, uint16_t bytes) {
  return apollo_do_i2c3_transaction(data, adr, bytes, I2C3_BUS_ZYNQ, I2C3_READ_TRANSACTION);
}
h7i2c_i2c_ret_code_t zynq_i2c_rx(uint8_t *data, uint8_t adr) {
  return zynq_i2c_rx_n(data, adr, 1);
}

/* Writes over the Zynq bus. */
h7i2c_i2c_ret_code_t zynq_i2c_tx_n(uint8_t *data, uint8_t adr, uint16_t bytes) {
  return apollo_do_i2c3_transaction(data, adr, bytes, I2C3_BUS_ZYNQ, I2C3_WRITE_TRANSACTION);
}
h7i2c_i2c_ret_code_t zynq_i2c_tx(uint8_t *data, uint8_t adr) {
  return zynq_i2c_tx_n(data, adr, 1);
}

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
 * Wrapper function that checks and clears the error state in the I2C3 peripheral 
 * and does the read/write transaction.
 */
static h7i2c_i2c_ret_code_t h7i2c_i2c_clear_error_state_and_do_transaction(h7i2c_periph_t peripheral, uint16_t dev_address, uint16_t data_size, uint8_t *data_buf, uint32_t timeout, i2c3_transaction_type_t transaction_type) {
  h7i2c_i2c_ret_code_t status = H7I2C_RET_CODE_OK;

  /* Before the read/write, check if peripheral is in error state. And if so, clear it. */
  status = h7i2c_i2c_check_and_clear_error_state(peripheral);
  
  /* If we could not clear the error state, do not proceed and return with the return code from the driver. */
  if (status != H7I2C_RET_CODE_OK) {
    return status;
  }

  /* Do the read/write. */
  if (transaction_type == I2C3_WRITE_TRANSACTION) {
    status = h7i2c_i2c_write(peripheral, dev_address, data_size, data_buf, timeout);
  }
  else if (transaction_type == I2C3_READ_TRANSACTION) {
    status = h7i2c_i2c_read(peripheral, dev_address, data_size, data_buf, timeout);
  }
  else {
    return H7I2C_RET_CODE_INVALID_ARGS;
  }

  /* Clear the error state in the peripheral again and return the status of the last transaction. */
  h7i2c_i2c_check_and_clear_error_state(peripheral);
  return status;
}

h7i2c_i2c_ret_code_t h7i2c_i2c_clear_error_state_and_read(h7i2c_periph_t peripheral, uint16_t dev_address, uint16_t data_size, uint8_t *data_buf, uint32_t timeout) {
  return h7i2c_i2c_clear_error_state_and_do_transaction(peripheral, dev_address, data_size, data_buf, timeout, I2C3_READ_TRANSACTION);
}

h7i2c_i2c_ret_code_t h7i2c_i2c_clear_error_state_and_write(h7i2c_periph_t peripheral, uint16_t dev_address, uint16_t data_size, uint8_t *data_buf, uint32_t timeout) {
  return h7i2c_i2c_clear_error_state_and_do_transaction(peripheral, dev_address, data_size, data_buf, timeout, I2C3_WRITE_TRANSACTION);
}