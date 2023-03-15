#ifndef ZYNQ_I2C_H_
#define ZYNQ_I2C_H_

#include "apollo_i2c.h"

uint8_t zynq_wr_reg(uint8_t adr, uint8_t data, uint8_t slave_addr);
uint8_t zynq_rd_reg(uint8_t adr, uint8_t slave_addr);

uint8_t zynq_get_i2c_done ();
uint8_t zynq_get_boot_error ();
uint8_t zynq_get_temperature();
void zynq_request_shutdown();

void zynq_set_ipmc_mac(uint8_t* mac);

void zynq_set_blade_sn(uint8_t sn);
void zynq_set_blade_slot(uint8_t slot);
void zynq_set_blade_rev(uint8_t rev);
void zynq_set_ipmc_ip(uint8_t* ip);
void zynq_set_ipmc_fw_hash(uint8_t* hash);
void zynq_set_ipmc_uid(uint8_t *uid);

void zynq_set_eth_mac(uint8_t eth, uint8_t *mac);
void zynq_set_eth_checksum(uint8_t eth, uint8_t checksum);

void zynq_set_s1_i2c_writes_done();

#endif // ZYNQ_I2C_H_
