#ifndef ZYNQ_I2C_H_
#define ZYNQ_I2C_H_

#include "apollo_i2c.h"

#define ZYNQ_I2C_SLAVE_ADDR 0x60
#define ZYNQ_I2C_DONE_REG 0

uint8_t get_zynq_rd (uint8_t adr);
uint8_t get_zynq_i2c_done ();

#endif // ZYNQ_I2C_H_
