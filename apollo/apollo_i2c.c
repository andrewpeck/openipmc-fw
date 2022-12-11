#include "apollo_i2c.h"
#include "sense_i2c.h"
#include "tca9546.h"

/*
 * These macros define I2C Tx, Rx and memRead functions for different
 * I2C buses on the Apollo. Single-byte and multi-byte reads and writes
 * are defined here.
 * 
 * I2C slave device address gets shifted by 1 for 7 bit vs. 8 bit 
 * addressing.
 */

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

#define CREATE_APOLLO_I2C_MEM_READ(FNAME, SEL) \
  HAL_StatusTypeDef FNAME ## _n (uint8_t *data, uint8_t slaveAddr, uint8_t memAddr, uint16_t bytes) { \
    HAL_StatusTypeDef stat = 0; \
    stat = SEL (); \
    stat = stat | sense_i2c_mem_read(slaveAddr<<1, memAddr, 1, data, bytes, 100); \
    return stat; \
  } \
  HAL_StatusTypeDef FNAME (uint8_t *data, uint8_t slaveAddr, uint8_t memAddr) { \
    return FNAME ## _n (data, slaveAddr, memAddr, 1); \
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
 * I2C mem-read functions for the CM1 and CM2 buses.
 * These have the repeated start conditions required by the CM MCU.
 */
CREATE_APOLLO_I2C_MEM_READ(cm1_i2c_mem_read,   tca9546_sel_m1);
CREATE_APOLLO_I2C_MEM_READ(cm2_i2c_mem_read,   tca9546_sel_m2);