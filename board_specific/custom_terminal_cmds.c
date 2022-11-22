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

/*
 * Multitask version for the original CLI_GetIntState() provided by terminal.
 *
 * Due to the multitask approach of this project, this function must be used
 * in the callbacks to test if ESC was pressed.
 */
extern bool mt_CLI_GetIntState();

static uint8_t apollo_restart_cb()
{
  mt_printf("\r\n");
  if (apollo_get_revision() == APOLLO_REV1) {
    mt_printf( "Restarting blade, network will disconnect\r\n\n" );
  }
  apollo_powerdown_sequence();
  osDelay(1000);
  apollo_powerup_sequence();
  return TE_OK;
}

static uint8_t apollo_powerdown_cb()
{
  mt_printf( "\r\n\n" );

  if (apollo_get_revision() != APOLLO_REV1) {
    apollo_powerdown_sequence();
  } else {
    mt_printf( "Powerdown not supported in Rev1.. please restart instead\r\n\n" );
  }
  return TE_OK;
}

static uint8_t apollo_powerup_cb()
{
  mt_printf( "\r\n\n" );
  if (apollo_get_revision() == APOLLO_REV1) {
    mt_printf( "Powering up, network will disconnect\r\n\n" );
  }
  apollo_powerup_sequence();
  return TE_OK;
}

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
    mt_printf("Boot mode = %d (%s)\r\n", mode, "JTAG");

  mt_printf("state = %s\r\n", get_apollo_status());

  mt_printf(" > zynq I2C 0x60  = %d\r\n", zynq_get_i2c_done());
  mt_printf(" > zynq FPGA DONE = %d\r\n", apollo_get_fpga_done ());
  mt_printf(" > zynq CPU Up    = %d\r\n", apollo_get_zynq_up ());

  return TE_OK;
}

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
  uint8_t adr = CLI_GetArgHex(0);
  uint8_t data = CLI_GetArgHex(1);

  HAL_StatusTypeDef status = HAL_OK;

  if (cm==1) {
    status |= cm1_i2c_tx(&adr, 0x40);
    status |= cm1_i2c_rx(&data, 0x40);
  }

  if (cm==2) {
    status |= cm2_i2c_tx(&adr, 0x40);
    status |= cm2_i2c_rx(&data, 0x40);
  }

  if (status==HAL_OK)
    mt_printf("CM%d I2C RX adr=0x%02X data=0x%02X\r\n", cm, adr, data);
  else
    mt_printf("I2C Failure\r\n");
  return status;
}

static uint8_t apollo_cm1_i2c_rx_cb() {
  return (apollo_cm_i2c_rx_cb(1));
}

static uint8_t apollo_cm2_i2c_rx_cb() {
  return(apollo_cm_i2c_rx_cb(2));
}

static uint8_t apollo_cm_i2c_tx_cb(uint8_t cm)
{
  mt_printf( "\r\n\n" );
  uint8_t adr = CLI_GetArgHex(0);
  uint8_t data = CLI_GetArgHex(1);

  HAL_StatusTypeDef status = HAL_OK;

  if (cm==1) {
    status |= cm1_i2c_tx(&adr, 0x40);
    status |= cm1_i2c_tx(&data, 0x40);
  }

  if (cm==2) {
    status |= cm2_i2c_tx(&adr, 0x40);
    status |= cm2_i2c_tx(&data, 0x40);
  }

  if (status==HAL_OK)
    mt_printf("CM%d I2C TX adr=0x%02X data=0x%02X\r\n", cm, adr, data);
  else
    mt_printf("I2C Failure\r\n");
  return status;
}

static uint8_t apollo_cm1_i2c_tx_cb() {
  return (apollo_cm_i2c_tx_cb(1));
}

static uint8_t apollo_cm2_i2c_tx_cb() {
  return(apollo_cm_i2c_tx_cb(2));
}


static uint8_t apollo_zynq_i2c_tx_cb()
{
  mt_printf( "\r\n\n" );
  uint8_t slave = CLI_GetArgHex(0);
  uint8_t adr = CLI_GetArgHex(1);
  uint8_t data = CLI_GetArgHex(2);

  uint8_t wr_data [] = {adr, data} ;

  HAL_StatusTypeDef status = HAL_OK;
  status |= zynq_i2c_tx_n (wr_data,  0x60+slave, 2);

  if (status==HAL_OK)
    mt_printf("Zynq I2C TX reg_adr=0x%02X data=0x%02X\r\n", adr, data);
  else
    mt_printf("I2C Failure\r\n");

  return status;
}

static uint8_t apollo_local_i2c_tx_cb()
{
  mt_printf( "\r\n\n" );
  uint8_t adr = CLI_GetArgHex(0);
  uint8_t data = CLI_GetArgHex(1);

  HAL_StatusTypeDef status = HAL_OK;
  status |= local_i2c_tx (&data,  adr);

  if (status==HAL_OK)
    mt_printf("Local I2C adr=0x%02X data=0x%02X\r\n", adr, data);
  else
    mt_printf("I2C Failure\r\n");
  return status;
}

void print_hal_status (HAL_StatusTypeDef status) {
  if (status==HAL_OK)
    mt_printf("HAL OK\r\n");
  else if (status==HAL_ERROR)
    mt_printf("HAL ERROR\r\n");
  else if (status==HAL_BUSY)
    mt_printf("HAL BUSY\r\n");
  else if (status==HAL_TIMEOUT)
    mt_printf("HAL TIMEOUT\r\n");
}

static uint8_t apollo_i2c_mux_cb()
{
  mt_printf( "\r\n\n" );

  uint8_t mask = CLI_GetArgHex(0) & 0xF;
  HAL_StatusTypeDef status = HAL_OK;
  status |= tca9546_config (mask);

  if (status==HAL_OK)
    mt_printf("I2C Mux Configured for 0x%1X\r\n", mask);
  else
    mt_printf("I2C Mux Failure\r\n");
  print_hal_status (status);

  return status;
}

static uint8_t apollo_read_pim_cb() {
  mt_printf("\r\n\n");

  uint8_t temp;
  uint8_t iout;
  uint8_t va;
  uint8_t vb;

  HAL_StatusTypeDef status = HAL_OK;
  status |= read_temp_pim400  (&temp);
  status |= read_iout_pim400  (&iout);
  status |= read_voltage_pim400  (&va, 0);
  status |= read_voltage_pim400  (&vb, 1);

  if (status==HAL_OK) {
    mt_printf("Temp=%d C\r\n", 2*temp-50);
    mt_printf("Iout=%f A\r\n", (94 * iout)/1000.0 );
    mt_printf("Va=%d V\r\n", (325*va)/1000);
    mt_printf("Vb=%d V\r\n", (325*vb)/1000);
  } else {
    mt_printf("I2C Read Failure\r\n");
  }
  return 0;
}


static uint8_t apollo_read_tcn_cb() {
  mt_printf("\r\n\n");

  for (int i=0; i<3; i++) {
    uint8_t rd;
    HAL_StatusTypeDef status = HAL_OK;
    status = read_sm_tcn(i, &rd);
    if (status==HAL_OK) {
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

static uint8_t apollo_write_ver_cb() {
  mt_printf("\r\n\n");
  uint8_t rev = CLI_GetArgDec(0);
  user_eeprom_set_version(rev);
  mt_printf("Setting EEPROM dataformat rev to = rev%d\r\n", rev);
  user_eeprom_write();
  mt_printf("EEPROM Read Back as:\r\n");
  return (apollo_read_eeprom_cb());
}


static uint8_t apollo_write_rev_cb() {
  mt_printf("\r\n\n");
  uint8_t rev = CLI_GetArgDec(0);
  user_eeprom_set_revision_number(rev);
  mt_printf("Setting EEPROM to = rev%d\r\n", rev);
  user_eeprom_write();
  mt_printf("EEPROM Read Back as:\r\n");
  return (apollo_read_eeprom_cb());
}


static uint8_t apollo_write_id_cb() {
  mt_printf("\r\n\n");
  uint8_t id = CLI_GetArgDec(0);
  user_eeprom_set_serial_number(id);
  mt_printf("Setting EEPROM to = id%d\r\n", id);
  user_eeprom_write();
  mt_printf("EEPROM Read Back as:\r\n");
  return (apollo_read_eeprom_cb());
}


static uint8_t apollo_local_i2c_rx_cb() {
  mt_printf("\r\n\n");

  uint8_t adr = CLI_GetArgHex(0);
  uint8_t data = 0x00;

  HAL_StatusTypeDef status = HAL_OK;
  status |= local_i2c_rx(&data, adr);

  if (status == HAL_OK)
    mt_printf("Local I2C RX reg_adr=0x%02X data=0x%02X\r\n", adr, data);
  else
    mt_printf("I2C Failure\r\n");
  return status;
}

static uint8_t apollo_zynq_i2c_rx_cb()
{
  mt_printf( "\r\n\n" );

  uint8_t slave = CLI_GetArgHex(0);
  uint8_t adr   = CLI_GetArgHex(1);
  uint8_t data  = 0xFF;

  HAL_StatusTypeDef status = HAL_OK;
  status |= zynq_i2c_tx (&adr,  0x60+slave);
  status |= zynq_i2c_rx (&data, 0x60+slave);

  if (status==HAL_OK)
    mt_printf("Zynq I2C RX reg_adr=0x%02X data=0x%02X\r\n", adr, data);
  else
    mt_printf("I2C Failure\r\n");
  return status;
}

static uint8_t apollo_write_eth_mac() {
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

static uint8_t apollo_cm1_i2c_addr_scan_cb()
{
  mt_printf( "\r\n\n" );

  // Scan every possible address on the CM1 I2C bus
  for (uint8_t addr=0x00, addr < 0xFF, addr++) {
    uint8_t status = 0;
    uint8_t data;
    status |= cm1_i2c_rx(&data, addr);
    mt_printf("Addr: %02X, Data: %02X, Status: %02X\r\n", addr, data, status);
  }
}

/*
 * This functions is called during terminal initialization to add custom
 * commands to the CLI by using CLI_AddCmd functions.
 *
 * Use the CLI_AddCmd() function according terminal documentation
 */
void add_board_specific_terminal_commands( void )
{
  // dashes and underscores don't seem to work as expected here :(
  CLI_AddCmd("bootmode",   apollo_boot_mode_cb,     1, 0, "Set the Apollo boot mode pin");
  CLI_AddCmd("bootstatus", apollo_boot_status_cb,   0, 0, "Get the status of the boot sequence");
  CLI_AddCmd("sdsel",      apollo_sdsel_cb,         1, 0, "Set the Apollo SD select pin");
  CLI_AddCmd("powerdown",  apollo_powerdown_cb,     0, 0, "Power down Apollo");
  CLI_AddCmd("powerup",    apollo_powerup_cb,       0, 0, "Power up Apollo");
  CLI_AddCmd("restart",    apollo_restart_cb,       0, 0, "Restart Apollo (may disconnect terminal)");
  CLI_AddCmd("readio",     apollo_read_io_cb,       0, 0, "Read IPMC status IOs");
  CLI_AddCmd("eepromrd",   apollo_read_eeprom_cb,   0, 0, "Read Apollo EEPROM");
  CLI_AddCmd("revwr",      apollo_write_rev_cb,     1, 0, "Write Apollo EEPROM Board Revision");
  CLI_AddCmd("idwr",       apollo_write_id_cb,      1, 0, "Write Apollo EEPROM Board ID");
  CLI_AddCmd("verwr",      apollo_write_ver_cb,     1, 0, "Write Apollo EEPROM Dataformat Revision");
  CLI_AddCmd("tcnrd",      apollo_read_tcn_cb,      0, 0, "Read Apollo TCN Temperature Sensors");
  CLI_AddCmd("pimrd",      apollo_read_pim_cb,      0, 0, "Read Apollo PIM400");

  CLI_AddCmd("i2csel",     apollo_i2c_mux_cb,       1, 0, "Configure Apollo I2C Mux");

  CLI_AddCmd("zwr",        apollo_zynq_i2c_tx_cb,   3, 0, "Write Apollo Zynq I2C");
  CLI_AddCmd("zrd",        apollo_zynq_i2c_rx_cb,   2, 0, "Read Apollo Zynq I2C");

  CLI_AddCmd("lwr",        apollo_local_i2c_tx_cb,  2, 0, "Write Apollo Local I2C");
  CLI_AddCmd("lrd",        apollo_local_i2c_rx_cb,  1, 0, "Read Apollo Local I2C");

  CLI_AddCmd("c1wr",       apollo_cm1_i2c_tx_cb,    2, 0, "Write Apollo CM1 I2C");
  CLI_AddCmd("c1rd",       apollo_cm1_i2c_rx_cb,    1, 0, "Read Apollo CM1 I2C");

  CLI_AddCmd("c2wr",       apollo_cm2_i2c_tx_cb,    2, 0, "Write Apollo CM2 I2C");
  CLI_AddCmd("c2rd",       apollo_cm2_i2c_rx_cb,    1, 0, "Read Apollo CM2 I2C");

  CLI_AddCmd("dis_shdn",   apollo_dis_shutoff_cb,   1, 0, "1 to disable IPMC shutdown if Zynq is not booted");

  CLI_AddCmd("ethmacwr",   apollo_write_eth_mac,    7, 0, "Set the ETH MAC address fields in EEPROM");

  CLI_AddCmd("i2cscan",    apollo_cm1_i2c_addr_scan_cb, 0, 0, "Do a scan of I2C devices on the CM1 I2C bus");
}
