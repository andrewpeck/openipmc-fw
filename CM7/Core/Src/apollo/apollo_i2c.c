#include "apollo_i2c.h"
#include "sense_i2c.h"
#include "tca9546.h"

// TCN75A         0b1001000 Top
// TCN75A         0b1001001 Bottom
// TCN75A         0b1001010 Center

#define CREATE_APOLLO_I2C_TX(FNAME, SEL) \
    HAL_StatusTypeDef FNAME (uint8_t *data, uint8_t adr) { \
      HAL_StatusTypeDef stat = 0; \
      stat = SEL (); \
      stat = stat | sense_i2c_transmit(adr, data, 1, 100); \
      return stat; \
    }

#define CREATE_APOLLO_I2C_RX(FNAME, SEL) \
    HAL_StatusTypeDef FNAME (uint8_t *data, uint8_t adr) { \
      HAL_StatusTypeDef stat = 0; \
      stat = SEL (); \
      stat = stat | sense_i2c_receive(adr, data, 1, 100); \
      return stat; \
    }

CREATE_APOLLO_I2C_TX (local_i2c_tx, tca9546_sel_local);
CREATE_APOLLO_I2C_TX (cm1_i2c_tx,   tca9546_sel_m1);
CREATE_APOLLO_I2C_TX (cm2_i2c_tx,   tca9546_sel_m2);
CREATE_APOLLO_I2C_TX (zynq_i2c_tx,  tca9546_sel_zynq);

CREATE_APOLLO_I2C_RX (local_i2c_rx, tca9546_sel_local);
CREATE_APOLLO_I2C_RX (cm1_i2c_rx,   tca9546_sel_m1);
CREATE_APOLLO_I2C_RX (cm2_i2c_rx,   tca9546_sel_m2);
CREATE_APOLLO_I2C_RX (zynq_i2c_rx,  tca9546_sel_zynq);

