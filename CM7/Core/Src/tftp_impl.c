
/*
 * This source implements the TFTP callbacks
 */

#include "tftp_client.h"

#include "write_bin_stmflash.h"


void tftp_close_cb(void* handle);
int  tftp_read_cb(void* handle, void* buf, int bytes);
int  tftp_write_cb(void* handle, struct pbuf* p);
void tftp_error_cb(void* handle, int err, const char* msg, int size);


struct tftp_context tftp_ctx = { .open  = NULL,
                                 .close = tftp_close_cb,
		                         .read  = tftp_read_cb,
		                         .write = tftp_write_cb,
		                         .error = tftp_error_cb   };


void tftp_close_cb(void* handle)
{
	bin_stmflash_close();
}

int tftp_read_cb(void* handle, void* buf, int bytes)
{
}

int tftp_write_cb(void* handle, struct pbuf* p)
{
	bin_stmflash_write( p->payload, p->len );

	return 0;
}

void tftp_error_cb(void* handle, int err, const char* msg, int size)
{
	mt_printf("TFTP ERROR.\r\n");
}




