#include "apollo_i2c.h"
#include "sense_i2c.h"
#include "tca9546.h"

// addr gets shifted by 1 for 7 bit vs. 8 bit addressing

#define CREATE_APOLLO_I2C_TX(FNAME, SEL) \
    HAL_StatusTypeDef FNAME ## _n (uint8_t *data, uint8_t adr, uint16_t bytes) { \
      HAL_StatusTypeDef stat = 0; \
      stat = SEL (); \
      stat = stat | sense_i2c_transmit(adr<<1, data, bytes, 100); \
      return stat; \
    } \
    HAL_StatusTypeDef FNAME (uint8_t *data, uint8_t adr) { \
      return FNAME ## _n (data, adr, 1); \
    } 

#define CREATE_APOLLO_I2C_RX(FNAME, SEL) \
    HAL_StatusTypeDef FNAME ## _n (uint8_t *data, uint8_t adr, uint16_t bytes) { \
      HAL_StatusTypeDef stat = 0; \
      stat = SEL (); \
      stat = stat | sense_i2c_receive(adr<<1, data, bytes, 100); \
      return stat; \
    } \
    HAL_StatusTypeDef FNAME (uint8_t *data, uint8_t adr) { \
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
 * @brief Perform a blocking read from a specific memory address on CM I2C target. 
 * 
 * @param data Pointer to data buffer
 * @param slaveAddr Target device address in 7-bits
 * @param memAddr Internal memory address to read in target device
 * @param bytes Number of bytes to read from I2C target memory
 * 
 * @retval HAL status 
 */
HAL_StatusTypeDef cm1_i2c_mem_read(uint8_t* data, uint8_t slaveAddr, uint8_t memAddr, uint16_t bytes)
{
  HAL_StatusTypeDef status = 0;
  // Select the CM1 bus
  status = tca9546_sel_m1();
  // Do the MemRead from the CM1 I2C target
  status = status | sense_i2c_mem_read(slaveAddr << 1, memAddr, 1, data, bytes, 100);
  return status;
}

