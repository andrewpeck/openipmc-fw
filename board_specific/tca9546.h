#ifndef __TCA9546_H
#define __TCA9546_H

#include "sense_i2c.h"

HAL_StatusTypeDef tca9546_config (uint8_t mask);
HAL_StatusTypeDef tca9546_sel_local ();
HAL_StatusTypeDef tca9546_sel_m1 ();
HAL_StatusTypeDef tca9546_sel_m2 ();
HAL_StatusTypeDef tca9546_sel_zynq ();

#endif
