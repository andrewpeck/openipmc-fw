#include "apollo_i2c.h"
#include "apollo_sensor_bus_mutex.h"
#include "tca9546.h"

/**
 * @brief Underlying function to call for an I2C read/write transaction over the sensor bus.
 * 
 * @param data                8-bit buffer for the data being read or written.
 * @param adr                 I2C address of the target device.
 * @param bytes               Number of bytes to read or write from the target device.
 * @param i2c3_bus            Enum representing which bus is picked for the transaction.
 * @param transaction_type    Enum representing if the transaction is a read or write. 
 */
static h7i2c_i2c_ret_code_t apollo_do_i2c_transaction_on_sensor_bus(uint8_t* data, uint8_t adr, uint16_t bytes, i2c3_bus_type_t i2c3_bus, i2c3_transaction_type_t transaction_type) {
  /* Take the mutex for sensor bus. If we fail to get the mutex, return BUSY status code. */
  if (apollo_sensor_bus_mutex_lock(100) != 0) { return H7I2C_RET_CODE_BUSY; }

  /* Write to the mux to pick the correct I2C bus. */
  h7i2c_i2c_ret_code_t status_mux = H7I2C_RET_CODE_OK;

  switch (i2c3_bus)
  {
  case SENSOR_BUS_LOCAL:
    status_mux = tca9546_sel_local();
    break;
  case SENSOR_BUS_CM1:
    status_mux = tca9546_sel_m1();
    break;
  case SENSOR_BUS_CM2:
    status_mux = tca9546_sel_m2();
    break;
  case SENSOR_BUS_ZYNQ:
    status_mux = tca9546_sel_zynq();
    break;
  default:
    return H7I2C_RET_CODE_INVALID_ARGS;
  }

  /* Check if the write to the mux went through. */
  if (status_mux != H7I2C_RET_CODE_OK) {
    return status_mux;
  }

  /* Do the actual write or read. Target device address is shifted by 1 byte due to 7-bit addressing. */
  h7i2c_i2c_ret_code_t status_io = H7I2C_RET_CODE_OK;
  if (transaction_type == APOLLO_I2C_BUS_WRITE_TRANSACTION) {
    status_io = h7i2c_i2c_clear_error_state_and_write(H7I2C_I2C3, adr<<1, bytes, data, 100);
  }
  else if (transaction_type == APOLLO_I2C_BUS_READ_TRANSACTION) {
    status_io = h7i2c_i2c_clear_error_state_and_read(H7I2C_I2C3, adr<<1, bytes, data, 100);
  }
  else {
    return H7I2C_RET_CODE_INVALID_ARGS;
  }

  /* Release the mutex for the sensor bus and return. */
  apollo_sensor_bus_mutex_release();
  return status_io;
}

/*
 * The reads/write functions (over the sensor bus) for the use of rest of the software are
 * defined below. Each read/write function has a multi-byte and a single-byte flavor.
 * 
 * These functions are wrappers to:
 * 1. Take the sensor bus mutex
 * 2. Write to the multiplexer
 * 3. Do the I2C read/write
 * 4. Release the sensor bus mutex
 */

/* Reads over the local I2C bus. */
h7i2c_i2c_ret_code_t local_i2c_rx_n(uint8_t *data, uint8_t adr, uint16_t bytes) {
  return apollo_do_i2c_transaction_on_sensor_bus(data, adr, bytes, SENSOR_BUS_LOCAL, APOLLO_I2C_BUS_READ_TRANSACTION);
}
h7i2c_i2c_ret_code_t local_i2c_rx(uint8_t *data, uint8_t adr) {
  return local_i2c_rx_n(data, adr, 1);
}

/* Writes over the local I2C bus. */
h7i2c_i2c_ret_code_t local_i2c_tx_n(uint8_t *data, uint8_t adr, uint16_t bytes) {
  return apollo_do_i2c_transaction_on_sensor_bus(data, adr, bytes, SENSOR_BUS_LOCAL, APOLLO_I2C_BUS_WRITE_TRANSACTION);
}
h7i2c_i2c_ret_code_t local_i2c_tx(uint8_t *data, uint8_t adr) {
  return local_i2c_tx_n(data, adr, 1);
}

/* Reads over the CM1 bus. */
h7i2c_i2c_ret_code_t cm1_i2c_rx_n(uint8_t *data, uint8_t adr, uint16_t bytes) {
  return apollo_do_i2c_transaction_on_sensor_bus(data, adr, bytes, SENSOR_BUS_CM1, APOLLO_I2C_BUS_READ_TRANSACTION);
}
h7i2c_i2c_ret_code_t cm1_i2c_rx(uint8_t *data, uint8_t adr) {
  return cm1_i2c_rx_n(data, adr, 1);
}

/* Writes over the CM1 bus. */
h7i2c_i2c_ret_code_t cm1_i2c_tx_n(uint8_t *data, uint8_t adr, uint16_t bytes) {
  return apollo_do_i2c_transaction_on_sensor_bus(data, adr, bytes, SENSOR_BUS_CM1, APOLLO_I2C_BUS_WRITE_TRANSACTION);
}
h7i2c_i2c_ret_code_t cm1_i2c_tx(uint8_t *data, uint8_t adr) {
  return cm1_i2c_tx_n(data, adr, 1);
}

/* Reads over the CM2 bus. */
h7i2c_i2c_ret_code_t cm2_i2c_rx_n(uint8_t *data, uint8_t adr, uint16_t bytes) {
  return apollo_do_i2c_transaction_on_sensor_bus(data, adr, bytes, SENSOR_BUS_CM2, APOLLO_I2C_BUS_READ_TRANSACTION);
}
h7i2c_i2c_ret_code_t cm2_i2c_rx(uint8_t *data, uint8_t adr) {
  return cm2_i2c_rx_n(data, adr, 1);
}

/* Writes over the CM1 bus. */
h7i2c_i2c_ret_code_t cm2_i2c_tx_n(uint8_t *data, uint8_t adr, uint16_t bytes) {
  return apollo_do_i2c_transaction_on_sensor_bus(data, adr, bytes, SENSOR_BUS_CM2, APOLLO_I2C_BUS_WRITE_TRANSACTION);
}
h7i2c_i2c_ret_code_t cm2_i2c_tx(uint8_t *data, uint8_t adr) {
  return cm2_i2c_tx_n(data, adr, 1);
}

/* Reads over the Zynq bus. */
h7i2c_i2c_ret_code_t zynq_i2c_rx_n(uint8_t *data, uint8_t adr, uint16_t bytes) {
  return apollo_do_i2c_transaction_on_sensor_bus(data, adr, bytes, SENSOR_BUS_ZYNQ, APOLLO_I2C_BUS_READ_TRANSACTION);
}
h7i2c_i2c_ret_code_t zynq_i2c_rx(uint8_t *data, uint8_t adr) {
  return zynq_i2c_rx_n(data, adr, 1);
}

/* Writes over the Zynq bus. */
h7i2c_i2c_ret_code_t zynq_i2c_tx_n(uint8_t *data, uint8_t adr, uint16_t bytes) {
  return apollo_do_i2c_transaction_on_sensor_bus(data, adr, bytes, SENSOR_BUS_ZYNQ, APOLLO_I2C_BUS_WRITE_TRANSACTION);
}
h7i2c_i2c_ret_code_t zynq_i2c_tx(uint8_t *data, uint8_t adr) {
  return zynq_i2c_tx_n(data, adr, 1);
}

/**
 * @brief Function to check the error state of the I2C peripheral, and clear it if necessary.
 * 
 * @param peripheral    Enum representing which I2C peripheral to operate on.
 */
h7i2c_i2c_ret_code_t h7i2c_i2c_check_and_clear_error_state(h7i2c_periph_t peripheral) {
  h7i2c_i2c_ret_code_t status = H7I2C_RET_CODE_OK;
  /* Clear the error state if this peripheral is in error state. */
  if (h7i2c_is_in_error(peripheral)) {
    status = h7i2c_clear_error_state(peripheral);
  }

  return status;
}

/**
 * @brief Wrapper function that checks and clears the error state in the given I2C peripheral and does the read/write transaction.
 *        After the read/write transaction, the error state in the peripheral is cleared again as a final step.
 * 
 * @param peripheral        Enum representing which I2C peripheral to operate on
 * @param dev_address       The address of the target device
 * @param data_size         Number of bytes of data to send or receive
 * @param data_buf          8-bit buffer for the incoming our outgoing data
 * @param timeout           Number of miliseconds to wait for a response from the target device before timing out
 * @param transaction_type  Enum representing whether the transaction is a read or write
 * 
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
  if (transaction_type == APOLLO_I2C_BUS_WRITE_TRANSACTION) {
    status = h7i2c_i2c_write(peripheral, dev_address, data_size, data_buf, timeout);
  }
  else if (transaction_type == APOLLO_I2C_BUS_READ_TRANSACTION) {
    status = h7i2c_i2c_read(peripheral, dev_address, data_size, data_buf, timeout);
  }
  else {
    return H7I2C_RET_CODE_INVALID_ARGS; /* Invalid transaction type. */
  }

  /* Clear the error state in the peripheral again and return the status of the last transaction. */
  h7i2c_i2c_check_and_clear_error_state(peripheral);
  return status;
}

/* Clear error state + read transaction. */
h7i2c_i2c_ret_code_t h7i2c_i2c_clear_error_state_and_read(h7i2c_periph_t peripheral, uint16_t dev_address, uint16_t data_size, uint8_t *data_buf, uint32_t timeout) {
  return h7i2c_i2c_clear_error_state_and_do_transaction(peripheral, dev_address, data_size, data_buf, timeout, APOLLO_I2C_BUS_READ_TRANSACTION);
}

/* Clear error state + write transaction. */
h7i2c_i2c_ret_code_t h7i2c_i2c_clear_error_state_and_write(h7i2c_periph_t peripheral, uint16_t dev_address, uint16_t data_size, uint8_t *data_buf, uint32_t timeout) {
  return h7i2c_i2c_clear_error_state_and_do_transaction(peripheral, dev_address, data_size, data_buf, timeout, APOLLO_I2C_BUS_WRITE_TRANSACTION);
}