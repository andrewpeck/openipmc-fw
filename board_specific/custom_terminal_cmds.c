#include "main.h"
#include "FreeRTOS.h"
#include "task.h"
#include "terminal.h"
#include "mt_printf.h"
#include "cmsis_os.h"
#include "cmsis_os2.h"

#include "apollo.h"
#include "apollo_i2c.h"
#include "sm_sensors.h"
#include "user_eeprom.h"
#include "zynq_i2c.h"
#include "tca9546.h"
#include "pim400.h"
#include "h7i2c_bare.h"

#include "h7i2c_bare.h"

/*
 * Multitask version for the original CLI_GetIntState() provided by terminal.
 *
 * Due to the multitask approach of this project, this function must be used
 * in the callbacks to test if ESC was pressed.
 */
extern bool mt_CLI_GetIntState();

#define COMMAND_APOLLO_RESTART_DESCRIPTION "\
Restart Apollo Service Module (may disconnect terminal). Usage: \r\n \
\t >> restart \r\n \
"
static uint8_t apollo_restart_cb()
{
  mt_printf("\r\n");
  if (apollo_get_revision() == APOLLO_REV1) {
    mt_printf( "Restarting blade, network will disconnect\r\n\n" );
  }
  apollo_powerdown_sequence(10);
  osDelay(1000);
  apollo_powerup_sequence();
  return TE_OK;
}

#define COMMAND_APOLLO_POWERDOWN_DESCRIPTION "\
Launch Apollo power down sequence. This is only supported in Service Modules of \r\n \
\t revision 2 or more recent. Usage: \r\n \
\t >> powerdown \r\n \
"
static uint8_t apollo_powerdown_cb()
{
  mt_printf( "\r\n\n" );

  if (apollo_get_revision() != APOLLO_REV1) {
    apollo_powerdown_sequence(10);
  } else {
    mt_printf( "Powerdown not supported in Rev1.. please restart instead\r\n\n" );
  }
  return TE_OK;
}

#define COMMAND_APOLLO_POWERUP_DESCRIPTION "\
Launch Apollo power up sequence. Usage: \r\n \
\t >> powerup \r\n \
"
static uint8_t apollo_powerup_cb()
{
  mt_printf( "\r\n\n" );
  if (apollo_get_revision() == APOLLO_REV1) {
    mt_printf( "Powering up, network will disconnect\r\n\n" );
  }
  apollo_powerup_sequence();
  return TE_OK;
}

#define COMMAND_APOLLO_READIO_DESCRIPTION "\
Read IPMC status IOs. Usage: \r\n \
\t >> readio \r\n"
static uint8_t apollo_read_io_cb()
{
  mt_printf( "\r\n\n" );
  mt_printf("Startup Done   = %d\r\n", apollo_get_ipmc_startup_done ());
  mt_printf("SM FPGA DONE   = %d\r\n", apollo_get_fpga_done ());
  mt_printf("SM CPU Up      = %d\r\n", apollo_get_zynq_up ());
  mt_printf("ESM Power Good = %d\r\n", apollo_get_esm_pwr_good ());
  mt_printf("Noshelf Jumper = %d\r\n", apollo_get_noshelf ());
  mt_printf("SM Revision    = %d\r\n", apollo_get_revision ());
  mt_printf("Boot Mode      = %d\r\n", apollo_get_zynq_boot_mode ());
  return TE_OK;
}

#define COMMAND_APOLLO_BOOT_STATUS_DESCRIPTION "\
Get the status of the Zynq boot sequence. Usage: \r\n \
\t >> bootstatus \r\n \
"
static uint8_t apollo_boot_status_cb()
{
  mt_printf( "\r\n\n" );

  uint8_t mode = apollo_get_zynq_boot_mode ();

  if (mode == APOLLO_BOOT_SD)
    mt_printf("Boot mode = %d (%s)\r\n", mode, "SD");
  else if (mode == APOLLO_BOOT_QSPI)
    mt_printf("Boot mode = %d (%s)\r\n", mode, "QSPI");
  else if (mode == APOLLO_BOOT_NAND)
    mt_printf("Boot mode = %d (%s)\r\n", mode, "NAND");
  else if (mode == APOLLO_BOOT_JTAG)
    mt_printf("Boot mode = %d (%s)\r\n", mode, "JTAG/EMMC");

  mt_printf("state = %s\r\n", get_apollo_status());

  mt_printf(" > zynq I2C 0x60  = %d\r\n", zynq_get_i2c_done());
  mt_printf(" > zynq FPGA DONE = %d\r\n", apollo_get_fpga_done ());
  mt_printf(" > zynq CPU Up    = %d\r\n", apollo_get_zynq_up ());

  return TE_OK;
}

#define COMMAND_APOLLO_EEPROMRD_DESCRIPTION "\
Read EEPROM contents on the Apollo. Usage: \r\n \
\t >> eepromrd \r\n \
"
static uint8_t apollo_read_eeprom_cb() {
  mt_printf("\r\n");

  osDelay(100);

  char status = user_eeprom_read();

  if (status == 0) {

    uint8_t boot_mode;
    uint8_t prom_rev;
    uint8_t rev;
    uint8_t id;
    uint8_t sdsel;
    uint8_t disable_shutoff;
    uint8_t eth0_mac_addr[6];
    uint8_t eth1_mac_addr[6];
    uint8_t eth0_mac_checksum;
    uint8_t eth1_mac_checksum;

    user_eeprom_get_revision_number(&rev);
    user_eeprom_get_serial_number(&id);
    user_eeprom_get_version(&prom_rev);
    user_eeprom_get_boot_mode(&boot_mode);
    user_eeprom_get_sdsel(&sdsel);
    user_eeprom_get_disable_shutoff(&disable_shutoff);

    user_eeprom_get_mac_addr(0, eth0_mac_addr);
    user_eeprom_get_mac_addr(1, eth1_mac_addr);

    user_eeprom_get_mac_eth_checksum(0, &eth0_mac_checksum);
    user_eeprom_get_mac_eth_checksum(1, &eth1_mac_checksum);

    // EEPROM versions 0 and 1 are valid now
    // Version 1 has the MAC address checksums included
    if ((prom_rev != 0x0) && (prom_rev != 0x1)) {
      mt_printf("WARNING! unknown prom version = 0x%02X; you should set the prom revision with `verwr 0/1`\r\n", prom_rev);
    }
    mt_printf("  prom version = 0x%02X\r\n", prom_rev);
    mt_printf("  bootmode     = 0x%02X\r\n", boot_mode);
    mt_printf("  sdsel        = 0x%02X\r\n", sdsel);
    mt_printf("  hw           = rev%d #%d\r\n", rev, id);
    mt_printf("  dis_shutoff  = 0x%02X\r\n", disable_shutoff);
    mt_printf("  eth0_mac     = ");
    
    for (uint8_t i=0; i<5; i++) {
      mt_printf("%02X:", eth0_mac_addr[i]);
    }
    mt_printf("%02X\r\n", eth0_mac_addr[5]);

    mt_printf("  eth1_mac     = ");
    for (uint8_t i=0; i<5; i++) {
      mt_printf("%02X:", eth1_mac_addr[i]);
    }
    mt_printf("%02X\r\n", eth1_mac_addr[5]);

    // Show the MAC address checksum values for revision 1
    if (prom_rev == 0x1) {
      mt_printf("  eth0_chsum   = 0x%02X\r\n", eth0_mac_checksum);
      mt_printf("  eth1_chsum   = 0x%02X\r\n", eth1_mac_checksum);
    }

  } else {
    mt_printf("I2C Failure Reading from EEPROM\r\n");
  }
  return status;
}

#define COMMAND_APOLLO_DIS_SHUTOFF_DESCRIPTION "\
Command to write 0x1 to disable IPMC shutdown if Zynq is not booted. \r\n \
\t >> Use dis_shdn 0x1 to disable IPMC shutdown. \r\n \
\t >> Use dis_shdn 0x0 to enable IPMC shutdown. \r\n \
"
static uint8_t apollo_dis_shutoff_cb()
{
  mt_printf( "\r\n\n" );
  uint8_t disable_shutoff = CLI_GetArgDec(0);
  if (disable_shutoff >= 0 && disable_shutoff <= 1) {
    mt_printf("Setting self shutoff mode to %d and saving in eeprom\r\n", disable_shutoff);
    user_eeprom_set_disable_shutoff(disable_shutoff);
    user_eeprom_write();
    return (apollo_read_eeprom_cb());
  }
  else {
    mt_printf("Invalid boot mode %d!\r\n", disable_shutoff);
    return TE_ArgErr;
  }
  return TE_OK;
}

#define COMMAND_SDSEL_DESCRIPTION "\
Set the Apollo pin for the SD card selection, to pick which SD card slot will be used to boot off Zynq from. \r\n \
\t Please note that this applies when the bootmode is set to 3. \r\n \
\t Usage: \r\n \
\t >> sdsel <slot> where slot is either 0 or 1. \r\n \
\t - Use sdsel 0 to boot off from the SD card on the back of the Service Module. \r\n \
\t - Use sdsel 1 to boot off from the SD card on the front-panel board slot. \r\n \
"
static uint8_t apollo_sdsel_cb()
{
  mt_printf( "\r\n\n" );
  uint8_t sdsel = CLI_GetArgDec(0);
  if (sdsel >= 0 && sdsel <= 1) {
    mt_printf("Setting sd select to %d and saving in eeprom\r\n", sdsel);
    apollo_set_sdsel(sdsel);
    user_eeprom_set_sdsel(sdsel);
    user_eeprom_write();
    return (apollo_read_eeprom_cb());
  }
  else {
    mt_printf("Invalid sd select %d!\r\n", sdsel);
    return TE_ArgErr;
  }
  return TE_OK;
}

#define COMMAND_BOOTMODE_DESCRIPTION "\
Set the Apollo boot mode pin, determining how the Zynq will boot. Usage: \r\n \
\t >> bootmode <BOOTMODE> where BOOTMODE is an integer between 0 and 3. \r\n \
\t - To boot Zynq from the SD card, use bootmode 3. \r\n \
\t - To boot Zynq from the EMMC,    use bootmode 0. \r\n \
"
static uint8_t apollo_boot_mode_cb()
{
  mt_printf( "\r\n\n" );
  uint8_t boot_mode = CLI_GetArgDec(0);
  if (boot_mode >= 0 && boot_mode <= 3) {
    mt_printf("Setting boot mode to %d and saving in eeprom\r\n", boot_mode);
    apollo_set_zynq_boot_mode(boot_mode);
    user_eeprom_set_boot_mode(boot_mode);
    user_eeprom_write();
    mt_printf("EEPROM Read Back as:\r\n");
    return (apollo_read_eeprom_cb());
  }
  else {
    mt_printf("Invalid boot mode %d!\r\n", boot_mode);
    return TE_ArgErr;
  }
  return TE_OK;
}

static uint8_t apollo_cm_i2c_rx_cb(uint8_t cm)
{
  mt_printf( "\r\n\n" );
  /* The register address to read from is the first argument to the CLI command. */
  uint8_t adr = CLI_GetArgHex(0);

  uint8_t rd_buf;

  /* 
   * Do the write and read with a repeated start condition. 
   * The microcontroller on the Command Module is at 0x40 on the bus.
   */
  h7i2c_i2c_ret_code_t status = H7I2C_RET_CODE_OK;
  uint8_t cm_mcu_adr = 0x40;

  if (cm==1) {
    status = cm1_i2c_tx_and_rx(&adr, &rd_buf, cm_mcu_adr);
  }
  else if (cm==2) {
    status = cm2_i2c_tx_and_rx(&adr, &rd_buf, cm_mcu_adr);
  }

  if (status==H7I2C_RET_CODE_OK)
    mt_printf("CM%d I2C RX adr=0x%02X data=0x%02X\r\n", cm, adr, rd_buf);
  else
    mt_printf("I2C Failure\r\n");
  return status;
}

#define COMMAND_CM1_I2C_RX_DESCRIPTION "\
Perform an I2C read over the CM1 bus. \r\n \
\t The target device is the CM microcontroller, located at address 0x40 by default. \r\n \
\t Usage: \r\n \
\t >> c1rd <regAdr> where: \r\n \
\t -regAdr : Address of the register to read. \r\n \
"
static uint8_t apollo_cm1_i2c_rx_cb() {
  return (apollo_cm_i2c_rx_cb(1));
}

#define COMMAND_CM2_I2C_RX_DESCRIPTION "\
Perform an I2C read over the CM2 bus. \r\n \
\t The target device is the CM microcontroller, located at address 0x40 by default. \r\n \
\t Usage: \r\n \
\t >> c2rd <regAdr> where: \r\n \
\t -regAdr : Address of the register to read. \r\n \
"
static uint8_t apollo_cm2_i2c_rx_cb() {
  return(apollo_cm_i2c_rx_cb(2));
}

static uint8_t apollo_cm_i2c_tx_cb(uint8_t cm)
{
  mt_printf( "\r\n\n" );
  uint8_t adr = CLI_GetArgHex(0);
  uint8_t data = CLI_GetArgHex(1);

  h7i2c_i2c_ret_code_t status = H7I2C_RET_CODE_OK;

  if (cm==1) {
    status |= cm1_i2c_tx(&adr, 0x40);
    status |= cm1_i2c_tx(&data, 0x40);
  }

  if (cm==2) {
    status |= cm2_i2c_tx(&adr, 0x40);
    status |= cm2_i2c_tx(&data, 0x40);
  }

  if (status==H7I2C_RET_CODE_OK)
    mt_printf("CM%d I2C TX adr=0x%02X data=0x%02X\r\n", cm, adr, data);
  else
    mt_printf("I2C Failure\r\n");
  return status;
}

#define COMMAND_CM1_I2C_TX_DESCRIPTION "\
Perform an I2C write over the CM1 bus. \r\n \
\t The target device is the CM microcontroller, located at address 0x40 by default. \r\n \
\t Usage: \r\n \
\t >> c1wr <regAdr> <data> where: \r\n \
\t -regAdr : Address of the register to write. \r\n \
\t -data   : 8-bit data to write. \r\n \
"
static uint8_t apollo_cm1_i2c_tx_cb() {
  return (apollo_cm_i2c_tx_cb(1));
}

#define COMMAND_CM2_I2C_TX_DESCRIPTION "\
Perform an I2C write over the CM2 bus. \r\n \
\t The target device is the CM microcontroller, located at address 0x40 by default. \r\n \
\t Usage: \r\n \
\t >> c2wr <regAdr> <data> where: \r\n \
\t -regAdr : Address of the register to write. \r\n \
\t -data   : 8-bit data to write. \r\n \
"
static uint8_t apollo_cm2_i2c_tx_cb() {
  return(apollo_cm_i2c_tx_cb(2));
}

#define COMMAND_ZYNQ_I2C_TX_DESCRIPTION "\
Perform an I2C write on the Zynq I2C bus. Usage:\r\n\
\t >> zwr <slaveNum> <regAdr> <data> where:\r\n\
\t -slaveNum : Number of the slave I2C device on Zynq (between 0-8).\r\n\
\t -regAdr   : Address of the register to write to on the slave device.\r\n\
\t -data     : The 8-bit data to write.\r\n\
"
static uint8_t apollo_zynq_i2c_tx_cb()
{
  mt_printf( "\r\n\n" );
  uint8_t slave = CLI_GetArgHex(0);
  uint8_t adr = CLI_GetArgHex(1);
  uint8_t data = CLI_GetArgHex(2);

  uint8_t wr_data [] = {adr, data} ;

  h7i2c_i2c_ret_code_t status = H7I2C_RET_CODE_OK;
  status |= zynq_i2c_tx_n (wr_data,  0x60+slave, 2);

  if (status==H7I2C_RET_CODE_OK)
    mt_printf("Zynq I2C TX reg_adr=0x%02X data=0x%02X\r\n", adr, data);
  else
    mt_printf("I2C Failure\r\n");

  return status;
}

#define COMMAND_LOCAL_I2C_TX_DESCRIPTION "\
Perform an I2C write on the local I2C bus. Usage: \r\n\
\t >> lwr <slaveAdr> <data> where:\r\n\
\t -slaveAdr : The I2C address of the target (slave) device. \r\n\
\t -data     : 8-bit data to write. \r\n\
"
static uint8_t apollo_local_i2c_tx_cb()
{
  mt_printf( "\r\n\n" );
  uint8_t adr = CLI_GetArgHex(0);
  uint8_t data = CLI_GetArgHex(1);

  h7i2c_i2c_ret_code_t status = H7I2C_RET_CODE_OK;
  status |= local_i2c_tx (&data,  adr);

  if (status==H7I2C_RET_CODE_OK)
    mt_printf("Local I2C adr=0x%02X data=0x%02X\r\n", adr, data);
  else
    mt_printf("I2C Failure\r\n");
  return status;
}

void print_hal_status (h7i2c_i2c_ret_code_t status) {
  if (status==H7I2C_RET_CODE_OK)
    mt_printf("HAL OK\r\n");
  else if (status==H7I2C_RET_CODE_ERROR)
    mt_printf("HAL ERROR\r\n");
  else if (status==H7I2C_RET_CODE_BUSY)
    mt_printf("HAL BUSY\r\n");
  else if (status==H7I2C_RET_CODE_TIMEOUT)
    mt_printf("HAL TIMEOUT\r\n");
}

#define COMMAND_APOLLO_I2C_MUX_DESCRIPTION "\
Set the value of the multiplexer (mux) on the I2C3 bus on Apollo. Usage:\r\n \
\t >> i2csel <busNum> where busNum is a 4-bit value representing which bus to pick. \r\n \
\t -To pick local bus, use: i2csel 0x1 \r\n \
\t -To pick CM1 bus,   use: i2csel 0x2 \r\n \
\t -To pick CM2 bus,   use: i2csel 0x4 \r\n \
\t -To pick Zynq bus,  use: i2csel 0x8 \r\n\n \
Please use the command with caution as other tasks running on the OpenIPMC (e.g., the sensor manager task) \r\n \
can interfere and write a different value to the mux. \r\n \
"
static uint8_t apollo_i2c_mux_cb()
{
  mt_printf( "\r\n\n" );

  uint8_t mask = CLI_GetArgHex(0) & 0xF;
  
  h7i2c_i2c_ret_code_t status = H7I2C_RET_CODE_OK;
  status |= tca9546_config (mask);

  if (status==H7I2C_RET_CODE_OK)
    mt_printf("I2C Mux Configured for 0x%1X\r\n", mask);
  else
    mt_printf("I2C Mux Failure\r\n");
  print_hal_status (status);

  return status;
}

#define COMMAND_APOLLO_READ_PIM_DESCRIPTION "\
Read Apollo PIM400 sensors. Usage: \r\n \
\t >> pimrd \r\n \
"
static uint8_t apollo_read_pim_cb() {
  mt_printf("\r\n\n");

  uint8_t temp;
  uint8_t iout;
  uint8_t va;
  uint8_t vb;

  h7i2c_i2c_ret_code_t status = H7I2C_RET_CODE_OK;
  status |= read_temp_pim400  (&temp);
  status |= read_iout_pim400  (&iout);
  status |= read_voltage_pim400  (&va, 0);
  status |= read_voltage_pim400  (&vb, 1);

  if (status==H7I2C_RET_CODE_OK) {
    mt_printf("Temp=%d C\r\n", 2*temp-50);
    mt_printf("Iout=%f A\r\n", (94 * iout)/1000.0 );
    mt_printf("Va=%d V\r\n", (325*va)/1000);
    mt_printf("Vb=%d V\r\n", (325*vb)/1000);
  } else {
    mt_printf("I2C Read Failure\r\n");
  }
  return 0;
}

#define COMMAND_APOLLO_READ_TCN_DESCRIPTION "\
Read Apollo TCN Temperature sensors. Usage: \r\n \
\t >> tcnrd \r\n \
"
static uint8_t apollo_read_tcn_cb() {
  mt_printf("\r\n\n");

  for (int i=0; i<3; i++) {
    uint8_t rd;
    h7i2c_i2c_ret_code_t status = H7I2C_RET_CODE_OK;
    status = read_sm_tcn(i, &rd);
    if (status==H7I2C_RET_CODE_OK) {
      if (i==TCN_TOP)
        mt_printf("Top=%d C\r\n", rd);
      if (i==TCN_MID)
        mt_printf("Mid=%d C\r\n", rd);
      if (i==TCN_BOT)
        mt_printf("Bot=%d C\r\n", rd);
    } else {
      mt_printf("I2C Read Failure\r\n");
    }
  }

  return 0;
}

#define COMMAND_APOLLO_WRITE_VER_DESCRIPTION "\
Write EEPROM dataformat revision on the EEPROM. Usage: \r\n \
\t >> verwr <verNum> where verNum can be 0 or 1. \r\n \
\t - verNum=1 is the latest revision which includes MAC address checksum (should be the default.) \r\n \
\t - verNum=0 is the older version without the MAC address checksum fields. \r\n \
"
static uint8_t apollo_write_ver_cb() {
  mt_printf("\r\n\n");
  uint8_t rev = CLI_GetArgDec(0);
  user_eeprom_set_version(rev);
  mt_printf("Setting EEPROM dataformat rev to = rev%d\r\n", rev);
  user_eeprom_write();
  mt_printf("EEPROM Read Back as:\r\n");
  return (apollo_read_eeprom_cb());
}


#define COMMAND_APOLLO_WRITE_REV_DESCRIPTION "\
Write Apollo Service Module revision on the EEPROM. Usage: \r\n \
\t >> revwr <revNum> where revNum is the numeric value representing the Service Module revision. \r\n \
\t -For rev1 SMs,           use: revwr 1 \r\n \
\t -For rev2 AND rev2a SMs, use: revwr 2 (please note we do not use '2a' for rev2a SMs, since expected value is numeric.) \r\n \
"
static uint8_t apollo_write_rev_cb() {
  mt_printf("\r\n\n");
  uint8_t rev = CLI_GetArgDec(0);
  user_eeprom_set_revision_number(rev);
  mt_printf("Setting EEPROM to = rev%d\r\n", rev);
  user_eeprom_write();
  mt_printf("EEPROM Read Back as:\r\n");
  return (apollo_read_eeprom_cb());
}

#define COMMAND_APOLLO_WRITE_ID_DESCRIPTION "\
Write Apollo board ID on EEPROM. Usage: \r\n \
\t >> idwr <apolloId> \r\n \
"
static uint8_t apollo_write_id_cb() {
  mt_printf("\r\n\n");
  uint8_t id = CLI_GetArgDec(0);
  user_eeprom_set_serial_number(id);
  mt_printf("Setting EEPROM to = id%d\r\n", id);
  user_eeprom_write();
  mt_printf("EEPROM Read Back as:\r\n");
  return (apollo_read_eeprom_cb());
}

#define COMMAND_LOCAL_I2C_RX_DESCRIPTION "\
Perform an I2C read on the local I2C bus. Usage: \r\n \
\t >> lrd <slaveAdr> where: \r\n \
\t -slaveAdr : The I2C address of the target (slave) device. \r\n \
"
static uint8_t apollo_local_i2c_rx_cb() {
  mt_printf("\r\n\n");

  uint8_t adr = CLI_GetArgHex(0);
  uint8_t data = 0x00;

  h7i2c_i2c_ret_code_t status = H7I2C_RET_CODE_OK;
  status |= local_i2c_rx(&data, adr);

  if (status == H7I2C_RET_CODE_OK)
    mt_printf("Local I2C RX reg_adr=0x%02X data=0x%02X\r\n", adr, data);
  else
    mt_printf("I2C Failure\r\n");
  return status;
}

#define COMMAND_ZYNQ_I2C_RX_DESCRIPTION "\
Perform an I2C read on the Zynq I2C bus. Usage:\r\n \
\t >> zrd <slaveNum> <regAdr> where: \r\n \
\t -slaveNum : Number of the slave I2C device on Zynq (between 0-8). \r\n \
\t -regAdr   : Address of the register to write to on the slave device. \r\n \
"
static uint8_t apollo_zynq_i2c_rx_cb()
{
  mt_printf( "\r\n\n" );

  uint8_t slave = CLI_GetArgHex(0);
  uint8_t adr   = CLI_GetArgHex(1);
  uint8_t data  = 0xFF;

  h7i2c_i2c_ret_code_t status = H7I2C_RET_CODE_OK;
  status |= zynq_i2c_tx (&adr,  0x60+slave);
  status |= zynq_i2c_rx (&data, 0x60+slave);

  if (status==H7I2C_RET_CODE_OK)
    mt_printf("Zynq I2C RX reg_adr=0x%02X data=0x%02X\r\n", adr, data);
  else
    mt_printf("I2C Failure\r\n");
  return status;
}

#define COMMAND_ETHMACWR_DESCRIPTION "\
Set the MAC address of the ETH0 or ETH1 port for the Zynq on EEPROM. Usage: \r\n \
\t >> ethmacwr <portNum> <MAC addr> where: \r\n \
\t -portNum  : 0 for eth0, 1 for eth1 port. (eth1 is the default for Zynqs on rev2a SMs) \r\n \
\t -MAC addr : Space separated MAC address. \r\n\n \
For example, to set a MAC address of 00:50:51:FF:10:CC on eth1 port, you can execute: \r\n \
\t >> ethmacwr 1 00 50 51 FF 10 CC \r\n \
"
static uint8_t apollo_write_eth_mac_cb() {
  /*
   * Sets the MAC address field in EEPROM. Also computes the two's complement
   of the checksum for the MAC address, and saves it on EEPROM.
   */
  mt_printf( "\r\n\n" );

  // First argument: Read the ETH port to set the MAC address for
  uint8_t eth = CLI_GetArgDec(0);

  // Read the MAC address one by one from the command line (and compute the checksum)
  uint8_t mac_adr[6];
  uint8_t checksum = 0;
  for (uint8_t i=0; i<6; i++) {
    mac_adr[i] = CLI_GetArgHex(i+1);
    checksum = (uint8_t) (checksum + mac_adr[i]);
  }

  // Set the MAC address in EEPROM
  user_eeprom_set_mac_addr(eth, mac_adr);
  mt_printf("Setting the MAC address for eth%d:\r\n", eth);
  for (int i=0; i<5; i++) {
    mt_printf("%02X:", mac_adr[i]);
  }
  mt_printf("%02X\r\n", mac_adr[5]);
  
  // Set the checksum for this ETH port, after computing two's complement of the sum
  uint8_t checksum_complement = (~checksum) + 1;
  user_eeprom_set_mac_eth_checksum(eth, checksum_complement);

  user_eeprom_write();
  mt_printf("EEPROM Read Back as:\r\n");
  return (apollo_read_eeprom_cb());

}

static uint8_t apollo_disable_i2c_interface_cb() {
  uint8_t peripheral_id = CLI_GetArgDec(0);

  switch (peripheral_id) {
    case 3:
      h7i2c_deinit(H7I2C_I2C3);
    case 4:
      h7i2c_deinit(H7I2C_I2C4);
    default:
      return 1;
  }

  return 0;
}

/*
 * This function is called during terminal initialization to add custom
 * commands to the CLI by using CLI_AddCmd functions.
 *
 * Use the CLI_AddCmd() function according terminal documentation.
 */
void add_board_specific_terminal_commands( void )
{
  // dashes and underscores don't seem to work as expected here :(
  CLI_AddCmd("bootmode",   apollo_boot_mode_cb,     1, 0, COMMAND_BOOTMODE_DESCRIPTION);
  CLI_AddCmd("bootstatus", apollo_boot_status_cb,   0, 0, COMMAND_APOLLO_BOOT_STATUS_DESCRIPTION);
  CLI_AddCmd("sdsel",      apollo_sdsel_cb,         1, 0, COMMAND_SDSEL_DESCRIPTION);
  CLI_AddCmd("powerdown",  apollo_powerdown_cb,     0, 0, COMMAND_APOLLO_POWERDOWN_DESCRIPTION);
  CLI_AddCmd("powerup",    apollo_powerup_cb,       0, 0, COMMAND_APOLLO_POWERUP_DESCRIPTION);
  CLI_AddCmd("restart",    apollo_restart_cb,       0, 0, COMMAND_APOLLO_RESTART_DESCRIPTION);
  CLI_AddCmd("readio",     apollo_read_io_cb,       0, 0, COMMAND_APOLLO_READIO_DESCRIPTION);
  CLI_AddCmd("eepromrd",   apollo_read_eeprom_cb,   0, 0, COMMAND_APOLLO_EEPROMRD_DESCRIPTION);
  CLI_AddCmd("revwr",      apollo_write_rev_cb,     1, 0, COMMAND_APOLLO_WRITE_REV_DESCRIPTION);
  CLI_AddCmd("idwr",       apollo_write_id_cb,      1, 0, COMMAND_APOLLO_WRITE_ID_DESCRIPTION);
  CLI_AddCmd("verwr",      apollo_write_ver_cb,     1, 0, COMMAND_APOLLO_WRITE_VER_DESCRIPTION);
  CLI_AddCmd("tcnrd",      apollo_read_tcn_cb,      0, 0, COMMAND_APOLLO_READ_TCN_DESCRIPTION);
  CLI_AddCmd("pimrd",      apollo_read_pim_cb,      0, 0, COMMAND_APOLLO_READ_PIM_DESCRIPTION);

  /* Mux configuration. */
  CLI_AddCmd("i2csel",     apollo_i2c_mux_cb,       1, 0, COMMAND_APOLLO_I2C_MUX_DESCRIPTION);

  /* Zynq I2C writes and reads. */
  CLI_AddCmd("zwr",        apollo_zynq_i2c_tx_cb,   3, 0, COMMAND_ZYNQ_I2C_TX_DESCRIPTION);
  CLI_AddCmd("zrd",        apollo_zynq_i2c_rx_cb,   2, 0, COMMAND_ZYNQ_I2C_RX_DESCRIPTION);

  /* Local bus I2C writes and reads. */
  CLI_AddCmd("lwr",        apollo_local_i2c_tx_cb,  2, 0, COMMAND_LOCAL_I2C_TX_DESCRIPTION);
  CLI_AddCmd("lrd",        apollo_local_i2c_rx_cb,  1, 0, COMMAND_LOCAL_I2C_RX_DESCRIPTION);

  /* CM1 bus I2C writes and reads. */
  CLI_AddCmd("c1wr",       apollo_cm1_i2c_tx_cb,    2, 0, COMMAND_CM1_I2C_TX_DESCRIPTION);
  CLI_AddCmd("c1rd",       apollo_cm1_i2c_rx_cb,    1, 0, COMMAND_CM1_I2C_RX_DESCRIPTION);

  /* CM2 bus I2C writes and reads. */
  CLI_AddCmd("c2wr",       apollo_cm2_i2c_tx_cb,    2, 0, COMMAND_CM2_I2C_TX_DESCRIPTION);
  CLI_AddCmd("c2rd",       apollo_cm2_i2c_rx_cb,    1, 0, COMMAND_CM2_I2C_RX_DESCRIPTION);

  CLI_AddCmd("dis_shdn",   apollo_dis_shutoff_cb,   1, 0, COMMAND_APOLLO_DIS_SHUTOFF_DESCRIPTION);

  /* Zynq MAC address write to the EEPROM. */
  CLI_AddCmd("ethmacwr",   apollo_write_eth_mac_cb, 7, 0, COMMAND_ETHMACWR_DESCRIPTION);

  CLI_AddCmd("i2cdisable", apollo_disable_i2c_interface_cb, 1, 0, "Disable the I2C3 or I2C4 interface.");
}
