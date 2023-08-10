#include "apollo_i2c.h"
#include "apollo_sensor_bus_mutex.h"
#include "tca9546.h"

/**
 * @brief Underlying function to call for an I2C read/write transaction over the sensor bus.
 * 
 * @param wr_data             8-bit buffer for the data being written.
 * @param rd_data             8-bit buffer for the data being read.
 * @param adr                 I2C address of the target device.
 * @param wr_bytes            Number of bytes to write to the target device.
 * @param rd_bytes            Number of bytes to read from the target device.
 * @param i2c3_bus            Enum representing which bus is picked for the transaction.
 */
static h7i2c_i2c_ret_code_t apollo_do_i2c_transaction_on_sensor_bus(uint8_t* wr_data, uint8_t* rd_data, uint16_t wr_bytes, uint16_t rd_bytes, uint8_t adr, i2c3_bus_type_t i2c3_bus) {
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

  /* Write transaction. */
  if ((rd_data == NULL) & (wr_data != NULL)) {
    status_io = h7i2c_i2c_clear_error_state_and_write(H7I2C_I2C3, adr<<1, wr_bytes, wr_data, 100);
  }
  /* Read transaction. */
  else if ((rd_data != NULL) & (wr_data == NULL)) {
    status_io = h7i2c_i2c_clear_error_state_and_read(H7I2C_I2C3, adr<<1, rd_bytes, rd_data, 100);
  }
  /* Write then read transaction with a repeated start. */
  else if ((rd_data != NULL) & (wr_data != NULL)) {
    status_io = h7i2c_i2c_clear_error_state_and_write_then_read(H7I2C_I2C3, adr<<1, wr_bytes, rd_bytes, wr_data, rd_data, 100);
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
  return apollo_do_i2c_transaction_on_sensor_bus(NULL, data, 0, bytes, adr, SENSOR_BUS_LOCAL);
}
h7i2c_i2c_ret_code_t local_i2c_rx(uint8_t *data, uint8_t adr) {
  return local_i2c_rx_n(data, adr, 1);
}

/* Writes over the local I2C bus. */
h7i2c_i2c_ret_code_t local_i2c_tx_n(uint8_t *data, uint8_t adr, uint16_t bytes) {
  return apollo_do_i2c_transaction_on_sensor_bus(data, NULL, bytes, 0, adr, SENSOR_BUS_LOCAL);
}
h7i2c_i2c_ret_code_t local_i2c_tx(uint8_t *data, uint8_t adr) {
  return local_i2c_tx_n(data, adr, 1);
}

/* Reads over the CM1 bus. */
h7i2c_i2c_ret_code_t cm1_i2c_rx_n(uint8_t *data, uint8_t adr, uint16_t bytes) {
  return apollo_do_i2c_transaction_on_sensor_bus(NULL, data, 0, bytes, adr, SENSOR_BUS_CM1);
}
h7i2c_i2c_ret_code_t cm1_i2c_rx(uint8_t *data, uint8_t adr) {
  return cm1_i2c_rx_n(data, adr, 1);
}

/* Writes over the CM1 bus. */
h7i2c_i2c_ret_code_t cm1_i2c_tx_n(uint8_t *data, uint8_t adr, uint16_t bytes) {
  return apollo_do_i2c_transaction_on_sensor_bus(data, NULL, bytes, 0, adr, SENSOR_BUS_CM1);
}
h7i2c_i2c_ret_code_t cm1_i2c_tx(uint8_t *data, uint8_t adr) {
  return cm1_i2c_tx_n(data, adr, 1);
}

h7i2c_i2c_ret_code_t cm1_i2c_tx_and_rx_n(uint8_t *wr_data, uint8_t *rd_data, uint16_t wr_bytes, uint16_t rd_bytes, uint8_t adr) {
  return apollo_do_i2c_transaction_on_sensor_bus(wr_data, rd_data, wr_bytes, rd_bytes, adr, SENSOR_BUS_CM1);
}
h7i2c_i2c_ret_code_t cm1_i2c_tx_and_rx(uint8_t *wr_data, uint8_t *rd_data, uint8_t adr) {
  return cm1_i2c_tx_and_rx_n(wr_data, rd_data, 1, 1, adr);
}

/* Reads over the CM2 bus. */
h7i2c_i2c_ret_code_t cm2_i2c_rx_n(uint8_t *data, uint8_t adr, uint16_t bytes) {
  return apollo_do_i2c_transaction_on_sensor_bus(NULL, data, 0, bytes, adr, SENSOR_BUS_CM2);
}
h7i2c_i2c_ret_code_t cm2_i2c_rx(uint8_t *data, uint8_t adr) {
  return cm2_i2c_rx_n(data, adr, 1);
}

/* Writes over the CM2 bus. */
h7i2c_i2c_ret_code_t cm2_i2c_tx_n(uint8_t *data, uint8_t adr, uint16_t bytes) {
  return apollo_do_i2c_transaction_on_sensor_bus(data, NULL, bytes, 0, adr, SENSOR_BUS_CM2);
}
h7i2c_i2c_ret_code_t cm2_i2c_tx(uint8_t *data, uint8_t adr) {
  return cm2_i2c_tx_n(data, adr, 1);
}

/* Reads over the Zynq bus. */
h7i2c_i2c_ret_code_t zynq_i2c_rx_n(uint8_t *data, uint8_t adr, uint16_t bytes) {
  return apollo_do_i2c_transaction_on_sensor_bus(NULL, data, 0, bytes, adr, SENSOR_BUS_ZYNQ);
}
h7i2c_i2c_ret_code_t zynq_i2c_rx(uint8_t *data, uint8_t adr) {
  return zynq_i2c_rx_n(data, adr, 1);
}

/* Writes over the Zynq bus. */
h7i2c_i2c_ret_code_t zynq_i2c_tx_n(uint8_t *data, uint8_t adr, uint16_t bytes) {
  return apollo_do_i2c_transaction_on_sensor_bus(data, NULL, bytes, 0, adr, SENSOR_BUS_ZYNQ);
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
 * @param wr_data_size      Number of bytes of data to send
 * @param rd_data_size      Number of bytes of data to receive
 * @param wr_data_buf       8-bit buffer for the data being written
 * @param rd_data_buf       8-bit buffer for the data being received
 * @param timeout           Number of miliseconds to wait for a response from the target device before timing out
 * @param transaction_type  Enum representing whether the transaction is a read or write
 * 
 */
static h7i2c_i2c_ret_code_t h7i2c_i2c_clear_error_state_and_do_transaction(h7i2c_periph_t peripheral, uint16_t dev_address, uint16_t wr_data_size, uint16_t rd_data_size, uint8_t *wr_data_buf, uint8_t *rd_data_buf, uint32_t timeout) {
  h7i2c_i2c_ret_code_t status = H7I2C_RET_CODE_OK;

  /* Before the read/write, check if peripheral is in error state. And if so, clear it. */
  status = h7i2c_i2c_check_and_clear_error_state(peripheral);
  
  /* If we could not clear the error state, do not proceed and return with the return code from the driver. */
  if (status != H7I2C_RET_CODE_OK) {
    return status;
  }

  /* Do the read/write. */
  if ((wr_data_buf != NULL) && (rd_data_buf == NULL)) {
    status = h7i2c_i2c_write(peripheral, dev_address, wr_data_size, wr_data_buf, timeout);
  }
  else if ((wr_data_buf == NULL) && (rd_data_buf != NULL)) {
    status = h7i2c_i2c_read(peripheral, dev_address, rd_data_size, rd_data_buf, timeout);
  }
  else if ((wr_data_buf != NULL) && (rd_data_buf != NULL)) {
    status = h7i2c_i2c_write_then_read(peripheral, dev_address, wr_data_size, rd_data_size, wr_data_buf, rd_data_buf, timeout);
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
  return h7i2c_i2c_clear_error_state_and_do_transaction(peripheral, dev_address, 0, data_size, NULL, data_buf, timeout);
}

/* Clear error state + write transaction. */
h7i2c_i2c_ret_code_t h7i2c_i2c_clear_error_state_and_write(h7i2c_periph_t peripheral, uint16_t dev_address, uint16_t data_size, uint8_t *data_buf, uint32_t timeout) {
  return h7i2c_i2c_clear_error_state_and_do_transaction(peripheral, dev_address, data_size, 0, data_buf, NULL, timeout);
}

/* Clear error state + write-then-read transaction. */
h7i2c_i2c_ret_code_t h7i2c_i2c_clear_error_state_and_write_then_read(h7i2c_periph_t peripheral, uint16_t dev_address, uint16_t wr_data_size, uint16_t rd_data_size, uint8_t *wr_data_buf, uint8_t *rd_data_buf, uint32_t timeout) {
  return h7i2c_i2c_clear_error_state_and_do_transaction(peripheral, dev_address, wr_data_size, rd_data_size, wr_data_buf, rd_data_buf, timeout);
}