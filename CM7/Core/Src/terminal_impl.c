
/********************************************************************************/
/*                                                                              */
/*    OpenIPMC-FW                                                               */
/*    Copyright (C) 2020-2021 Andre Cascadan, Luigi Calligaris                  */
/*                                                                              */
/*    This program is free software: you can redistribute it and/or modify      */
/*    it under the terms of the GNU General Public License as published by      */
/*    the Free Software Foundation, either version 3 of the License, or         */
/*    (at your option) any later version.                                       */
/*                                                                              */
/*    This program is distributed in the hope that it will be useful,           */
/*    but WITHOUT ANY WARRANTY; without even the implied warranty of            */
/*    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             */
/*    GNU General Public License for more details.                              */
/*                                                                              */
/*    You should have received a copy of the GNU General Public License         */
/*    along with this program.  If not, see <https://www.gnu.org/licenses/>.    */
/*                                                                              */
/********************************************************************************/

/*
 * This source is dedicated to implement CLI commands and associated resources.
 */

// FreeRTOS includes
#include "FreeRTOS.h"
#include "stream_buffer.h"
#include "semphr.h"

// Resources for terminal
#include "main.h"
#include "printf.h"
#include "string.h"
#include "terminal.h"

// Resources for commands
#include "ipmb_0.h"
#include "head_commit_sha1.h"
#include "fru_state_machine.h"
#include "device_id.h"
#include "cmsis_os.h"
#include "apollo.h"
#include "apollo_i2c.h"
#include "sm_sensors.h"
#include "user_eeprom.h"
#include "zynq_i2c.h"
#include "tca9546.h"
#include "pim400.h"
#include "tftp_client.h"
#include "write_bin_stmflash.h"
#include "bootloader_tools.h"

StreamBufferHandle_t terminal_input_stream = NULL;
SemaphoreHandle_t    terminal_semphr       = NULL;

/*
 * Multitask version for the original CLI_CheckAbort() provided by "terminal".
 *
 * Due to the multitask approach of this project, this macro must be used.
 */
#define mt_CLI_CheckAbort()                           \
{                                                     \
	xSemaphoreTake( terminal_semphr, portMAX_DELAY ); \
	CLI_CheckAbort();                                 \
}

static int esc_translator( char* c, _Bool esc_timeout );





// Command configuration
#define CMD_INFO_NAME "info"
#define CMD_INFO_DESCRIPTION "Print information about this IPMC."
#define CMD_INFO_CALLBACK info_cb

#define CMD_ATCA_HANDLE_NAME "handle"
#define CMD_ATCA_HANDLE_DESCRIPTION "\
Force a state change for ATCA Face Plate Handle.\r\n\
\t[ o | c ] for Open or Close."
#define CMD_ATCA_HANDLE_CALLBACK atca_handle_cb

#define CMD_DEBUG_IPMI_NAME "debug-ipmi"
#define CMD_DEBUG_IPMI_DESCRIPTION "\
Enable to show the IPMI messaging from OpenIPMC"
#define CMD_DEBUG_IPMI_CALLBACK debug_ipmi_cb

#define CMD_LOAD_BIN_NAME "load-bin"
#define CMD_LOAD_BIN_DESCRIPTION "\
Load binary from a TFTP server into TEMP area (Sector 12).\r\n\
\t\tex: load-bin 192 168 0 1 new_firmware.bin\r\n\
\t\tThis client always load file named fimware.bin"
#define CMD_LOAD_BIN_CALLBACK load_bin_cb

#define CMD_CHECK_BIN_NAME "check-bin"
#define CMD_CHECK_BIN_DESCRIPTION "\
Check the validity of binary present in Sector 12."
#define CMD_CHECK_BIN_CALLBACK check_bin_cb

#define CMD_BOOT_NAME "bootloader"
#define CMD_BOOT_DESCRIPTION "\
Manage Bootloader.\r\n\
\t\tNo argument: Print Bootloader status\r\n\
\t\t     enable: Bootloader boots first after reset\r\n\
\t\t    disable: OpenIPMC-FW boots directly after reset"
#define CMD_BOOT_CALLBACK bootloader_cb

#define CMD_RESET_NAME "reset"
#define CMD_RESET_DESCRIPTION "\
Reset IPMC. Use this instead \"~\"!"
#define CMD_RESET_CALLBACK reset_cb


/*
 * Callback for "info"
 *
 * Prints general information about IPMC
 */
static uint8_t info_cb()
{
	// Convert commit hash from hex to string.
	char commit_s[9];
	sprintf_(commit_s, "%08x", HEAD_COMMIT_SHA1);

	mt_printf( "\r\n\n" );
	mt_printf( "OpenIPMC-HW\r\n" );
	mt_printf( "Firmware commit: %s\r\n", commit_s );
	mt_printf( "\r\n" );
	mt_printf( "Target Board: %s\r\n", ipmc_device_id.device_id_string );
	mt_printf( "IPMB-0 Addr: 0x%x\r\n", ipmb_0_addr );
	return TE_OK;
}

/*
 * Callback for "handle"
 *
 * Force a state change for ATCA Face Plate Handle
 */
static uint8_t atca_handle_cb()
{
	fru_transition_t fru_trigg_val;

	if ( CLI_IsArgFlag("-o") )
	{
		fru_trigg_val = OPEN_HANDLE;
		xQueueSendToBack(queue_fru_transitions, &fru_trigg_val, 0UL);
	}
	else if ( CLI_IsArgFlag("-c") ){
		fru_trigg_val = CLOSE_HANDLE;
		xQueueSendToBack(queue_fru_transitions, &fru_trigg_val, 0UL);
	}

	return TE_OK;
}

static uint8_t apollo_powerdown_cb()
{
	mt_printf( "\r\n\n" );
	mt_printf("Powering down service module\r\n");
	apollo_powerdown_sequence();
	return TE_OK;
}

static uint8_t apollo_powerup_cb()
{
	mt_printf( "\r\n\n" );
	mt_printf("Powering up service module\r\n");
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

static uint8_t apollo_disable_shutoff_cb()
{
	mt_printf( "\r\n\n" );
	uint8_t disable_shutoff = CLI_GetArgDec(0);
	if (disable_shutoff >= 0 && disable_shutoff <= 1) {
		mt_printf("Setting self shutoff mode to %d and saving in eeprom\r\n", disable_shutoff);
		user_eeprom_set_disable_shutoff(disable_shutoff);
		user_eeprom_write();
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
	uint8_t adr = CLI_GetArgHex(0);
	uint8_t data = CLI_GetArgHex(1);

	HAL_StatusTypeDef status = HAL_OK;
	status |= zynq_i2c_tx (&adr,  0x60);
	status |= zynq_i2c_tx (&data, 0x60);

	if (status==HAL_OK)
		mt_printf("Zynq I2C TX adr=0x%02X data=0x%02X\r\n", adr, data);
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

static uint8_t apollo_read_eeprom_cb() {
	mt_printf("\r\n\n");

	char status = user_eeprom_read();

	if (status == 0) {

		uint8_t boot_mode;
		uint8_t prom_rev;
		uint8_t rev;
		uint8_t id;
		uint8_t sdsel;

		user_eeprom_get_revision_number(&rev);
		user_eeprom_get_serial_number(&id);
		user_eeprom_get_version(&prom_rev);
		user_eeprom_get_boot_mode(&boot_mode);
		user_eeprom_get_sdsel(&sdsel);

		mt_printf("prom version = 0x%02X\r\n", prom_rev);
		mt_printf("bootmode     = 0x%02X\r\n", boot_mode);
		mt_printf("sdsel        = 0x%02X\r\n", sdsel);
		mt_printf("hw           = rev%d #%d\r\n", rev, id);
	} else {
		mt_printf("I2C Failure\r\n");
	}
	return status;
}
static uint8_t apollo_write_rev_cb() {
		mt_printf("\r\n\n");
		uint8_t rev = CLI_GetArgDec(0);
		user_eeprom_set_revision_number(rev);
		mt_printf("Setting EEPROM to = rev%d\r\n", rev);
		user_eeprom_write();
		osDelay(100);
		mt_printf("EEPROM Read Back as:\r\n");
		return (apollo_read_eeprom_cb());
}


static uint8_t apollo_write_id_cb() {
		mt_printf("\r\n\n");
		uint8_t id = CLI_GetArgDec(0);
		user_eeprom_set_serial_number(id);
		mt_printf("Setting EEPROM to = id%d\r\n", id);
		user_eeprom_write();
		osDelay(100);
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

	uint8_t adr = CLI_GetArgHex(0);
	uint8_t data = 0xFF;

	HAL_StatusTypeDef status = HAL_OK;
	status |= zynq_i2c_tx (&adr,  0x60);
	status |= zynq_i2c_rx (&data, 0x60);

	if (status==HAL_OK)
		mt_printf("Zynq I2C RX reg_adr=0x%02X data=0x%02X\r\n", adr, data);
	else
		mt_printf("I2C Failure\r\n");
	return status;
}

/*
 * Callback for "debug-ipmi"
 *
 * Enable to show the IPMI messaging from OpenIPMC
 */
extern int enable_ipmi_printouts;
static uint8_t debug_ipmi_cb()
{
	enable_ipmi_printouts = 1; // Enable Debug

	while(1)
	{
		// Wait for ESC
		vTaskDelay( pdMS_TO_TICKS( 500 ) );
		xSemaphoreTake( terminal_semphr, portMAX_DELAY );
		if( CLI_GetIntState() ){
			enable_ipmi_printouts = 0; // Disable debug
			break;
		}
	}

	return TE_WorkInt;
}


_Bool tftp_impl_fwupload_start( const ip_addr_t *server_addr, const char* fname );
_Bool tftp_impl_fwupload_running( void );
void  tftp_impl_fwupload_abort( void );

uint8_t load_bin_cb()
{
	// Get Server IP addr from parameters
	ip_addr_t tfpt_server_addr;
	IP_ADDR4(&tfpt_server_addr, CLI_GetArgDec(0)&0xFF, CLI_GetArgDec(1)&0xFF, CLI_GetArgDec(2)&0xFF, CLI_GetArgDec(3)&0xFF);

	// Get filename from parameters
	char* filename = CLI_GetArgv()[5];

	mt_printf( "\r\n" );

	if( tftp_impl_fwupload_start( &tfpt_server_addr, filename ) )
	{
		while( tftp_impl_fwupload_running() )
		{
			vTaskDelay( pdMS_TO_TICKS( 500 ) );
			xSemaphoreTake( terminal_semphr, portMAX_DELAY );
			if( CLI_GetIntState() ) // If ESC is pressed
			{
				tftp_impl_fwupload_abort();
				mt_printf( "Transfer Aborted!" );
				return TE_OK;
			}
		}
	}
	else
		mt_printf( "Transfer Failed!" );


	return TE_OK;
}


static uint8_t check_bin_cb()
{
	uint32_t crc;
	int is_valid = bin_stmflash_validate(9, &crc);
	mt_printf( "\r\n");
	if( is_valid == 0 )
		mt_printf( "Binary is invalid!\r\n" );
	mt_printf( "CRC: %x\r\n", crc );

	return TE_OK;
}

static uint8_t bootloader_cb()
{
	uint8_t major_ver; // (version info not used here)
	uint8_t minor_ver;
	uint8_t aux_ver[4];
	
	mt_printf( "\r\n\r\n" );

	if ( CLI_IsArgFlag("enable") )
	{
		if( bootloader_enable() == false )
			mt_printf( "Enabling failed!\r\n" );
	}
	else if( CLI_IsArgFlag("disable") )
	{
		if( bootloader_disable() == false )
			mt_printf( "Disabling failed!\r\n" );
	}

	mt_printf( "Bootloader is present in the Flash: " );
	if( bootloader_is_present( &major_ver, &minor_ver, aux_ver ) )
		mt_printf( "YES  ver:%d.%d.%d  %02x%02x%02x%02x\r\n", major_ver, (minor_ver>>4)&0x0F, (minor_ver)&0x0F, aux_ver[0], aux_ver[1], aux_ver[2], aux_ver[3] );
	else
		mt_printf( "NO\r\n" );

	mt_printf( "Run bootloader on boot is enabled: " );
	if( bootloader_is_active() )
		mt_printf( "YES\r\n" );
	else
		mt_printf( "NO\r\n" );

	return TE_OK;
}


/*
 * Callback for "~"
 *
 * Reboots the MCU. Command defined natively by terminal
 */
void _reset_fcn( void )
{
	// Do not implement "~" for RESET
}
static uint8_t reset_cb()
{
	NVIC_SystemReset();
}





/*
 * Task for feeding characters to the terminal
 *
 * The CLI_EnterChar() calls printf internally. Sinsce the corrent implementation
 * of printf uses FreeRTOS resources, it can not be called from an interrupt.
 * Therefore, a dedicated task need to be used for that.
 */
void terminal_input_task(void *argument)
{
	char c[3];     // Size 3 is required by the ESC translator
	int rcvd_ctr;
	int trans_ctr;

	// Wait for resources
	while( ( terminal_input_stream == NULL ) ||
	       ( terminal_semphr       == NULL )    )
		vTaskDelay( pdMS_TO_TICKS( 500 ) );

	while(1)
	{
		rcvd_ctr = xStreamBufferReceive( terminal_input_stream, &c[0], 1, pdMS_TO_TICKS( 500 ) );

		if( rcvd_ctr != 0 )
			// Character was received normally. Send it to the translator.
			trans_ctr = esc_translator( c, false );
		else
			// On Timeout occurred. Just inform the translator
			trans_ctr = esc_translator( c, true );

		// Send the translated characters to the terminal
		xSemaphoreTake( terminal_semphr, 0 );

		for( int i=0; i<trans_ctr; ++i )
			CLI_EnterChar( c[i] );

		xSemaphoreGive( terminal_semphr );


	}
}


/*
 * Task for processing the CLI.
 *
 * The commands called on the terminal will also be run in the context of
 * this task, since the command callbacks are called CLI_Execute() when
 * a command is recognised from the terminal.
 *
 * This task also initializes the terminal and other resources
 */
void terminal_process_task(void *argument)
{
	terminal_input_stream = xStreamBufferCreate(10, 1);
	terminal_semphr = xSemaphoreCreateBinary();

	CLI_Init(TDC_None);

	// Define the commands to the CLI
	CLI_AddCmd( CMD_INFO_NAME,        CMD_INFO_CALLBACK,        0, 0, CMD_INFO_DESCRIPTION        );
	CLI_AddCmd( CMD_ATCA_HANDLE_NAME, CMD_ATCA_HANDLE_CALLBACK, 1, 0, CMD_ATCA_HANDLE_DESCRIPTION );
	CLI_AddCmd( CMD_DEBUG_IPMI_NAME,  CMD_DEBUG_IPMI_CALLBACK,  0, 0, CMD_DEBUG_IPMI_DESCRIPTION  );
	//CLI_AddCmd( CMD_LOAD_BIN_NAME,    CMD_LOAD_BIN_CALLBACK,    5, TMC_None, CMD_LOAD_BIN_DESCRIPTION    );
	//CLI_AddCmd( CMD_CHECK_BIN_NAME,   CMD_CHECK_BIN_CALLBACK,   0, TMC_None, CMD_CHECK_BIN_DESCRIPTION   );
	CLI_AddCmd( CMD_BOOT_NAME,        CMD_BOOT_CALLBACK,        0, TMC_None, CMD_BOOT_DESCRIPTION   );
    CLI_AddCmd( CMD_RESET_NAME,       CMD_RESET_CALLBACK,       0, TMC_None, CMD_RESET_DESCRIPTION   );


	// dashes and underscores don't seem to work as expected here :(
	CLI_AddCmd("bootmode",   apollo_boot_mode_cb,     1, 0, "Set the apollo boot mode pin");
	CLI_AddCmd("bootstatus", apollo_boot_status_cb,   0, 0, "Get the status of the boot sequence");
	CLI_AddCmd("sdsel",      apollo_sdsel_cb,         1, 0, "Set the apollo sd select pin");
	CLI_AddCmd("powerdown",  apollo_powerdown_cb,     0, 0, "Power down Apollo");
	CLI_AddCmd("powerup",    apollo_powerup_cb,       0, 0, "Power up Apollo");
	CLI_AddCmd("readio",     apollo_read_io_cb,       0, 0, "Read IPMC status IOs");
	CLI_AddCmd("eepromrd",   apollo_read_eeprom_cb,   0, 0, "Read Apollo EEPROM");
	CLI_AddCmd("revwr",      apollo_write_rev_cb,     1, 0, "Write Apollo EEPROM Revision");
	CLI_AddCmd("idwr",       apollo_write_id_cb,      1, 0, "Write Apollo EEPROM Revision");
	CLI_AddCmd("tcnrd",      apollo_read_tcn_cb,      0, 0, "Read Apollo TCN Temperature Sensors");
	CLI_AddCmd("pimrd",      apollo_read_pim_cb,      0, 0, "Read Apollo PIM400");

	CLI_AddCmd("i2csel",     apollo_i2c_mux_cb,       1, 0, "Configure Apollo I2C Mux");

	CLI_AddCmd("zynqwr",     apollo_zynq_i2c_tx_cb,   1, 0, "Write Apollo Zynq I2C");
	CLI_AddCmd("zynqrd",     apollo_zynq_i2c_rx_cb,   1, 0, "Read Apollo Zynq I2C");

	CLI_AddCmd("localwr",    apollo_local_i2c_tx_cb,  1, 0, "Write Apollo Local I2C");
	CLI_AddCmd("localrd",    apollo_local_i2c_rx_cb,  1, 0, "Read Apollo Local I2C");

	CLI_AddCmd("cm1wr",      apollo_cm1_i2c_tx_cb,    1, 0, "Write Apollo CM1 I2C");
	CLI_AddCmd("cm1rd",      apollo_cm1_i2c_rx_cb,    1, 0, "Read Apollo CM1 I2C");

	CLI_AddCmd("cm2wr",      apollo_cm2_i2c_tx_cb,    1, 0, "Write Apollo CM2 I2C");
	CLI_AddCmd("cm2rd",      apollo_cm2_i2c_rx_cb,    1, 0, "Read Apollo CM2 I2C");

	CLI_AddCmd("disableshutoff", apollo_disable_shutoff_cb, 1, 0, "1 to disable IPMC shutdown if Zynq is not booted");

	// Andre recommended commenting this out for now, due to a known bug
	//info_cb();
	
	while(1)
	{
		xSemaphoreTake( terminal_semphr, portMAX_DELAY );
		CLI_Execute();
	}
}


/*
 * Translator. It analyzes input sequences an translate the expected non ascii keys
 */
static int esc_translator( char* c, _Bool esc_timeout )
{
	static char buff[2];
	static int  ctr = 0;
	int ret;
	if( !esc_timeout )
	{
		if( (ctr == 0) && (c[0] != '\e') ) // Normal case: common character
			return 1;

		else if( (ctr == 0) && (c[0] == '\e') ) //received ESC: stores it
		{
			buff[ctr++] = '\e';
			return 0;
		}
		else if( ctr == 1 ) // Second char: just store
		{
			buff[ctr++] = c[0];
			return 0;
		}
		else if( ctr == 2 ) // Third char: analyze key
		{
			if( (buff[1] == '[') && ( c[0] == 'A') )// Arrow Up
			{
				c[0] = TERM_KEY_UP;
				ctr = 0;
				return 1;
			}
			if( (buff[1] == '[') && ( c[0] == 'B') )// Arrow Down
			{
				c[0] = TERM_KEY_DOWN;
				ctr = 0;
				return 1;
			}
			if( (buff[1] == '[') && ( c[0] == 'C') )// Arrow Right
			{
				c[0] = TERM_KEY_RIGHT;
				ctr = 0;
				return 1;
			}
			if( (buff[1] == '[') && ( c[0] == 'D') )// Arrow Left
			{
				c[0] = TERM_KEY_LEFT;
				ctr = 0;
				return 1;
			}

			else // No pattern found: just dump
			{
				c[2] = c[0];
				c[0] = buff[0];
				c[1] = buff[1];
				ctr = 0;
				return 3;
			}
		}
	}
	else // If timeout with 2 or less chars, just dump the buffer and restarts.
	{
		c[0] = buff[0];
		c[1] = buff[1];
		ret = ctr;
		ctr = 0;
		return ret;
	}

	return 0;
}
