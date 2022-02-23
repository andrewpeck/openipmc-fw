#include "sense_i2c.h"
#include "tca9546.h"
#include "apollo.h"

uint8_t config_last = 0x00;

// TODO: should keep track of the current switch state and only change if it
// needs to

HAL_StatusTypeDef tca9546_config(uint8_t mask) {
  if (mask != config_last) {
    config_last = mask;
    const uint8_t tca9546_adr = 0x70 << 1;
    uint8_t tx_data = mask & 0xf;
    return sense_i2c_transmit(tca9546_adr, &tx_data, 1, 2000);
  }
  else {
    return HAL_OK;
  }

}

HAL_StatusTypeDef tca9546_sel_local () {
  return tca9546_config (0x1);
}

HAL_StatusTypeDef tca9546_sel_m1() {
  if (GET_12V_STATE == 1) {
    return tca9546_config(0x2);
  } else {
    return tca9546_config(0x0);
  }
}

HAL_StatusTypeDef tca9546_sel_m2() {
  if (GET_12V_STATE == 1) {
    return tca9546_config(0x4);
  } else {
    return tca9546_config(0x0);
  }
}

HAL_StatusTypeDef tca9546_sel_zynq() {
  if (GET_12V_STATE == 1) {
    return tca9546_config(0x8);
  } else {
    return tca9546_config(0x0);
  }
}
