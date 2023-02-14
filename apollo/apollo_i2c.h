#ifndef APOLLO_I2C_H
#define APOLLO_I2C_H

#include "sense_i2c.h"

// single byte flavors of i2c tx/rx
HAL_StatusTypeDef local_i2c_tx (uint8_t *data, uint8_t adr);
HAL_StatusTypeDef local_i2c_rx (uint8_t *data, uint8_t adr);
HAL_StatusTypeDef cm1_i2c_tx (uint8_t *data, uint8_t adr);
HAL_StatusTypeDef cm1_i2c_rx (uint8_t *data, uint8_t adr);
HAL_StatusTypeDef cm2_i2c_tx (uint8_t *data, uint8_t adr);
HAL_StatusTypeDef cm2_i2c_rx (uint8_t *data, uint8_t adr);
HAL_StatusTypeDef zynq_i2c_tx (uint8_t *data, uint8_t adr);
HAL_StatusTypeDef zynq_i2c_rx (uint8_t *data, uint8_t adr);

// multi-byte flavors of i2c tx/rx
HAL_StatusTypeDef local_i2c_tx_n (uint8_t *data, uint8_t adr, uint16_t bytes);
HAL_StatusTypeDef local_i2c_rx_n (uint8_t *data, uint8_t adr, uint16_t bytes);
HAL_StatusTypeDef cm1_i2c_tx_n (uint8_t *data, uint8_t adr, uint16_t bytes);
HAL_StatusTypeDef cm1_i2c_rx_n (uint8_t *data, uint8_t adr, uint16_t bytes);
HAL_StatusTypeDef cm2_i2c_tx_n (uint8_t *data, uint8_t adr, uint16_t bytes);
HAL_StatusTypeDef cm2_i2c_rx_n (uint8_t *data, uint8_t adr, uint16_t bytes);
HAL_StatusTypeDef zynq_i2c_tx_n (uint8_t *data, uint8_t adr, uint16_t bytes);
HAL_StatusTypeDef zynq_i2c_rx_n (uint8_t *data, uint8_t adr, uint16_t bytes);

#endif /* APOLLO_I2C_H */
