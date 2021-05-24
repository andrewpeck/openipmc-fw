#ifndef APOLLO_I2C_H
#define APOLLO_I2C_H

#include "sense_i2c.h"

HAL_StatusTypeDef local_i2c_tx (uint8_t *data, uint8_t adr);
HAL_StatusTypeDef local_i2c_rx (uint8_t *data, uint8_t adr);
HAL_StatusTypeDef cm1_i2c_tx (uint8_t *data, uint8_t adr);
HAL_StatusTypeDef cm1_i2c_rx (uint8_t *data, uint8_t adr);
HAL_StatusTypeDef cm2_i2c_tx (uint8_t *data, uint8_t adr);
HAL_StatusTypeDef cm2_i2c_rx (uint8_t *data, uint8_t adr);
HAL_StatusTypeDef zynq_i2c_tx (uint8_t *data, uint8_t adr);
HAL_StatusTypeDef zynq_i2c_rx (uint8_t *data, uint8_t adr);

#endif /* APOLLO_I2C_H */
