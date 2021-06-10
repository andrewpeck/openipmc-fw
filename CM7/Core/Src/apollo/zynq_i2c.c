#include "zynq_i2c.h"

uint8_t zynq_rd_reg (uint8_t adr) {

  uint8_t data = adr;

  HAL_StatusTypeDef status = HAL_OK;
  status |= zynq_i2c_tx (&data, ZYNQ_I2C_SLAVE_ADDR);
  status |= zynq_i2c_rx (&data, ZYNQ_I2C_SLAVE_ADDR);

  if (status)
    return 0;
  else
    return data;

}

uint8_t get_zynq_i2c_done () {
  return zynq_rd_reg (ZYNQ_I2C_DONE_REG);
}
