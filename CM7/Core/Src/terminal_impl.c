


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
#include "st_bootloader.h"
#include "device_id.h"



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


// Command configuration
#define CMD_INFO_NAME "info"
#define CMD_INFO_DESCRIPTION "Print information about this IPMC."
#define CMD_INFO_CALLBACK info_cb

#define CMD_ATCA_HANDLE_NAME "handle"
#define CMD_ATCA_HANDLE_DESCRIPTION "\
Force a state change for ATCA Face Plate Handle.\r\n\
\t[ o | c ] for Open or Close."
#define CMD_ATCA_HANDLE_CALLBACK atca_handle_cb

#define CMD_ST_BOOT_NAME "st-boot"
#define CMD_ST_BOOT_DESCRIPTION "\
Reboot and jump to STMicroelectronics bootloader"
#define CMD_ST_BOOT_CALLBACK st_boot_cb



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
 * Callback for "st-boot"
 *
 * Reboot and jump to STMicroelectronics bootloader
 */
static uint8_t st_boot_cb()
{
	st_bootloader_launch();

	return TE_OK;
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
	char c;

	// Wait for resources
	while( ( terminal_input_stream == NULL ) ||
	       ( terminal_semphr       == NULL )    )
		vTaskDelay( pdMS_TO_TICKS( 500 ) );

	while(1)
	{
		if( xStreamBufferReceive( terminal_input_stream, &c, 1, portMAX_DELAY ) != 0 )
		{
			xSemaphoreTake( terminal_semphr, 0 );
			CLI_EnterChar(c);
			xSemaphoreGive( terminal_semphr );
		}
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
	CLI_AddCmd( CMD_ST_BOOT_NAME,     CMD_ST_BOOT_CALLBACK,     0, 0, CMD_ST_BOOT_DESCRIPTION     );


	while(1)
	{
		xSemaphoreTake( terminal_semphr, portMAX_DELAY );
		CLI_Execute();
	}
}


/*
 * Callback for "~"
 *
 * Reboots the MCU. Command defined natively by terminal
 */
void _reset_fcn( void )
{
	NVIC_SystemReset();
}
