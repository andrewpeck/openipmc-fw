
/*
 * Implements tools to perform firmware upload via TFTP
 *
 * Dedicated for uploading the new OpenIPMC-FW when remote
 * update is desired
 *
 * The uploaded firmware is written in the TEMP_SECTOR of
 * STM32 internal Flash
 */

#include <stdbool.h>

#include "FreeRTOS.h"
#include "task.h"

#include "tftp_client.h"
#include "write_bin_stmflash.h"


#define TFTP_FWUP_PORT 69
#define TEMP_SECTOR    9   // Flash sector where firmware start to be written


// Transfer control
static _Bool transfer_running_flag;


static void tftp_fwupload_close_cb(void* handle);
static int  tftp_fwupload_read_cb(void* handle, void* buf, int bytes);
static int  tftp_fwupload_write_cb(void* handle, struct pbuf* p);
static void tftp_fwupload_error_cb(void* handle, int err, const char* msg, int size);


static struct tftp_context tftp_ctx_fwupload =
{
	.open  = NULL,
	.close = tftp_fwupload_close_cb,
	.read  = tftp_fwupload_read_cb,
	.write = tftp_fwupload_write_cb,
	.error = tftp_fwupload_error_cb
};


_Bool tftp_impl_fwupload_start( const ip_addr_t *server_addr, const char* fname )
{
	int dummy_handle;

	// Clear control flags
	transfer_running_flag  = false;

	bin_stmflash_open(TEMP_SECTOR);
	tftp_init_client(&tftp_ctx_fwupload);
	tftp_get( &dummy_handle, server_addr, TFTP_FWUP_PORT, fname, TFTP_MODE_OCTET );

	// Timeout control (4 seconds)
	_Bool timed_out = true;
	for(int i=0; i<8; i++)
	{
		vTaskDelay( pdMS_TO_TICKS( 500 ) );
		if( transfer_running_flag )
		{
			timed_out = false;
			break;
		}
	}

	if(timed_out)
	{
		tftp_cleanup();
		transfer_running_flag  = false;
		return 0;
	}
	else
		return 1;
}

_Bool tftp_impl_fwupload_running( void )
{
	return transfer_running_flag;
}

void tftp_impl_fwupload_abort( void )
{
	tftp_cleanup();
	transfer_running_flag = false;
}



// TFTP callbacks

static void tftp_fwupload_close_cb(void* handle)
{
	bin_stmflash_close();
	transfer_running_flag = false;
}

static int tftp_fwupload_read_cb(void* handle, void* buf, int bytes)
{
}

static int tftp_fwupload_write_cb(void* handle, struct pbuf* p)
{
	transfer_running_flag = true;
	bin_stmflash_write( p->payload, p->len );

	return 0;
}

static void tftp_fwupload_error_cb(void* handle, int err, const char* msg, int size)
{
	mt_printf("TFTP ERROR.\r\n");
}




