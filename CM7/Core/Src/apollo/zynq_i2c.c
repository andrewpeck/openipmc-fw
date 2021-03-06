#include "zynq_i2c.h"

// Documentation here:
// https://apollo-lhc.gitlab.io/IPMC/ipmc-sensors/#zynq-i2c-slave-interface

#define ZYNQ_I2C_SLAVE1_ADDR 0x60
#define ZYNQ_I2C_SLAVE8_ADDR 0x67

#define ZYNQ_I2C_DONE_REG 0

#define IPMC_MAC_ADR_REG 0x00
#define IPMC_IP_ADR_REG 0x08

//------------------------------------------------------------------------------
// Generic rd/wr functions
//------------------------------------------------------------------------------

uint8_t zynq_rd_reg(uint8_t adr, uint8_t slave_addr) {

  uint8_t data = adr;

  HAL_StatusTypeDef status = HAL_OK;
  status |= zynq_i2c_tx(&data, slave_addr);
  status |= zynq_i2c_rx(&data, slave_addr);

  if (status)
    return 0;
  else
    return data;
}

uint8_t zynq_wr_reg(uint8_t adr, uint8_t data, uint8_t slave_addr) {

  uint8_t wr_data [] = {adr, data} ;
  HAL_StatusTypeDef status = HAL_OK;
  status |= zynq_i2c_tx_n(wr_data, slave_addr, 2);

  if (status)
    return 0;
  else
    return 1;
}

//------------------------------------------------------------------------------
// Slave specific wr/rd functions
//------------------------------------------------------------------------------

uint8_t zynq_s1_wr_reg(uint8_t adr, uint8_t data) {
  return zynq_wr_reg(adr, data, ZYNQ_I2C_SLAVE1_ADDR);
}

uint8_t zynq_s8_wr_reg(uint8_t adr, uint8_t data) {
  return zynq_wr_reg(adr, data, ZYNQ_I2C_SLAVE8_ADDR);
}

uint8_t zynq_s1_rd_reg(uint8_t adr) {
  return zynq_rd_reg(adr, ZYNQ_I2C_SLAVE1_ADDR);
}

uint8_t zynq_s8_rd_reg(uint8_t adr) {
  return zynq_rd_reg(adr, ZYNQ_I2C_SLAVE8_ADDR);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------

uint8_t zynq_get_i2c_done() {
  return (0x1 & zynq_s1_rd_reg(ZYNQ_I2C_DONE_REG));
}

uint8_t zynq_get_boot_error() {
  return (0x2 & zynq_s1_rd_reg(ZYNQ_I2C_DONE_REG));
}

void zynq_request_shutdown() {
  uint8_t rd = zynq_s1_rd_reg(ZYNQ_I2C_DONE_REG);
  rd = rd | 0x10;
  zynq_s1_wr_reg(ZYNQ_I2C_DONE_REG, rd);
}

void zynq_set_blade_sn(uint8_t sn) {
  zynq_s1_wr_reg(0x4, sn);
}

void zynq_set_blade_slot(uint8_t slot) {
  zynq_s1_wr_reg(0x5, slot);
}

void zynq_set_blade_rev(uint8_t rev) {
  zynq_s1_wr_reg(0x7, rev);
}

void zynq_set_ipmc_ip(uint8_t *ip) {
  for (int i=0; i<4; i++) {
    zynq_s8_wr_reg(IPMC_IP_ADR_REG+i, ip[i]);
  }
}

void zynq_set_ipmc_mac(uint8_t *mac) {
  for (int i=0; i<6; i++) {
    zynq_s8_wr_reg(IPMC_MAC_ADR_REG+i, mac[i]);
  }
}
