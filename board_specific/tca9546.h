#ifndef __TCA9546_H
#define __TCA9546_H

#include "sense_i2c.h"
#include "h7i2c_bare.h"

h7i2c_i2c_ret_code_t tca9546_config (uint8_t mask);
h7i2c_i2c_ret_code_t tca9546_sel_local ();
h7i2c_i2c_ret_code_t tca9546_sel_m1 ();
h7i2c_i2c_ret_code_t tca9546_sel_m2 ();
h7i2c_i2c_ret_code_t tca9546_sel_zynq ();

#endif
