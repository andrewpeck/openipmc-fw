#include "zynq_i2c.h"

// Documentation here:
// https://apollo-lhc.gitlab.io/IPMC/ipmc-sensors/#zynq-i2c-slave-interface

#define ZYNQ_I2C_SLAVE1_ADDR 0x60
#define ZYNQ_I2C_SLAVE6_ADDR 0x65
#define ZYNQ_I2C_SLAVE8_ADDR 0x67

#define ZYNQ_I2C_DONE_REG 0

// Read/write targets for SLAVE8 target on the Zynq
#define IPMC_MAC_ADR_REG 0x00
#define IPMC_IP_ADR_REG 0x08
#define IPMC_FW_HASH_REG 0x10
#define IPMC_UID_REG 0x14

// Read/write target addresses for SLAVE6 target on the Zybq
#define IPMC_SITE_NUMBER_ADDR 0x0
#define IPMC_SHELF_ID_ADDR 0x4

// Read/write targets for SLAVE1 target on the Zynq
#define ZYNQ_ETH0_MAC_ADR_REG 0x14


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

uint8_t zynq_s6_wr_reg(uint8_t adr, uint8_t data) {
  return zynq_wr_reg(adr, data, ZYNQ_I2C_SLAVE6_ADDR);
}

uint8_t zynq_s1_rd_reg(uint8_t adr) {
  return zynq_rd_reg(adr, ZYNQ_I2C_SLAVE1_ADDR);
}

uint8_t zynq_s8_rd_reg(uint8_t adr) {
  return zynq_rd_reg(adr, ZYNQ_I2C_SLAVE8_ADDR);
}

uint8_t zynq_s6_rd_reg(uint8_t adr) {
  return zynq_rd_reg(adr, ZYNQ_I2C_SLAVE6_ADDR);
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

void zynq_set_ipmc_fw_hash(uint8_t *hash) {
  /*
   * Writes the IPMC FW hash to the Zynq registers, 8 bits at a time. 
   */
  for (int i=0; i<4; i++) {
    zynq_s8_wr_reg(IPMC_FW_HASH_REG+i, hash[i]);
  }
}

void zynq_set_ipmc_uid(uint8_t *uid) {
  /*
   * Send the unique device ID for this IPMC to Zynq.
   * Device ID is 32-bits, send 8-bit at a time.
   */
  for (int i=0; i<4; i++) {
    zynq_s8_wr_reg(IPMC_UID_REG+i, uid[i]);
  }
}

void zynq_set_ipmc_mac(uint8_t *mac) {
  for (int i=0; i<6; i++) {
    zynq_s8_wr_reg(IPMC_MAC_ADR_REG+i, mac[i]);
  }
}

void zynq_set_eth_mac(uint8_t eth, uint8_t *mac) {
  // Address of BYTE_0 for: 
  // ETH0 port -> 0x14
  // ETH1 port -> 0x1C
  uint8_t base_adr = ZYNQ_ETH0_MAC_ADR_REG + eth * 8;
  for (int i=0; i<6; i++) {
    uint8_t adr = base_adr + i;
    zynq_s1_wr_reg(adr, mac[i]);
  }
}

void zynq_set_eth_checksum(uint8_t eth, uint8_t checksum) {
  // Write the checksum to the adjacent byte of the MAC address
  uint8_t adr = (ZYNQ_ETH0_MAC_ADR_REG + eth * 8) + 6;
  zynq_s1_wr_reg(adr, checksum);
}

void zynq_set_s1_i2c_writes_done()
  /*
   * Write a 1 value to Zynq's I2C register indicating that all the I2C
   * writes to the S1 I2C slave are complete.
   */
{
  uint8_t rd = zynq_s1_rd_reg(ZYNQ_I2C_DONE_REG);

  // The mask of the I2C_DONE register is 0x8 
  rd = rd | 0x8;
  zynq_s1_wr_reg(ZYNQ_I2C_DONE_REG, rd);
}

uint8_t zynq_get_temperature()
/*
 * Function to read the temperature value from Zynq S1 I2C slave.
 */
{
  uint8_t reg_adr = 0x24;
  return zynq_rd_reg(reg_adr, ZYNQ_I2C_SLAVE1_ADDR);
}

void zynq_set_site_number(uint8_t site_number) {
  /*
   * Write IPMC site number to S6.
   */
  zynq_s6_wr_reg(IPMC_SITE_NUMBER_ADDR, site_number);
}

void zynq_set_shelf_id(uint8_t *shelf_id) {
  /*
   * Write shelf ID data to S6 target device. The shelf ID data is 20 bytes,
   * we'll do the write one byte at a time.
   */
  for (int i=0; i<20; i++) {
    zynq_s6_wr_reg(IPMC_SHELF_ID_ADDR+i, shelf_id[i]);
  }
}