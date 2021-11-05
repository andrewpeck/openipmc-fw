
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
