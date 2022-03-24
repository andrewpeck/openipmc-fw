
#include "main.h"
#include "FreeRTOS.h"
#include "task.h"
#include "terminal.h"
#include "mt_printf.h"


/*
 * Multitask version for the original CLI_GetIntState() provided by terminal.
 *
 * Due to the multitask approach of this project, this function must be used
 * in the callbacks to test if ESC was pressed.
 */
extern bool mt_CLI_GetIntState();



static uint8_t example_cb()
{
	mt_printf( "\r\n" );

	while( !mt_CLI_GetIntState() )
	{
		mt_printf( "This is the EXAMPLE command. Press ESC to quit.\r\n" );
		vTaskDelay( pdMS_TO_TICKS( 500 ) );
	}

	return TE_OK;
}


/*
 * This functions is called during terminal initialization to add custom
 * commands to the CLI by using CLI_AddCmd functions.
 *
 * Use the CLI_AddCmd() function according terminal documentation
 */
void add_board_specific_terminal_commands( void )
{
	CLI_AddCmd( "example", example_cb, 0, TMC_None, "Run EXAMPLE command" );
}
