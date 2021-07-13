#include "sense_i2c.h"
#include "tca9546.h"

HAL_StatusTypeDef tca9546_config (uint8_t mask) {
  const uint8_t tca9546_adr = 0x70 << 1;
  uint8_t tx_data = mask & 0xf;
  return sense_i2c_transmit(tca9546_adr, &tx_data, 1, 2000);
}

HAL_StatusTypeDef tca9546_sel_local () {
  return tca9546_config (0x1);
}

HAL_StatusTypeDef tca9546_sel_m1 () {
  return tca9546_config (0x2);
}

HAL_StatusTypeDef tca9546_sel_m2 () {
  return tca9546_config (0x4);
}

HAL_StatusTypeDef tca9546_sel_zynq () {
  if (GET_12V_STATE == 1 ) {
    return tca9546_config (0x8);
  }
  else  {
    return tca9546_config (0x0);
  }
}
