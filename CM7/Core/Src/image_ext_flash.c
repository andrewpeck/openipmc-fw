
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
 * This file implements dedicated functions to write and read firmware images
 * on the external flash memory (Winbond W25N01GV).
 */

#include "main.h"

#include "FreeRTOS.h"
#include "task.h"

#include "w25n01gv.h"

// Flash characteristics
#define BLOCK_SIZE   131072   // Size of erasable block: 128KB or 64 pages = 131072 bytes
#define PAGE_SIZE      2048   // Flash is programmed in pages of 2048 bytes

static uint32_t write_origin;
static uint32_t write_index;

// Buffer in RAM for 2048 bytes flash page
// TODO move this buffer to another bigger and slower region
static uint8_t page_buffer[PAGE_SIZE];
static int     page_index;


extern CRC_HandleTypeDef hcrc;


void image_ext_flash_open( int block_number )
{
	write_origin  = block_number * BLOCK_SIZE;
	write_index   = 0;
	page_index    = 0;

	// Initialize flash
	w25n01gv_device_reset();
	w25n01gv_set_status_reg(0xA0, 0x00);

	// Erase first used block
	w25n01gv_write_enable();
	w25n01gv_block_erase( block_number );
	while( w25n01gv_is_busy() )
		vTaskDelay( pdMS_TO_TICKS(50) );
}

void image_ext_flash_write( uint8_t* data, int len )
{

	for( int i=0; i<len; i++)
	{
		if( page_index <  PAGE_SIZE)
		{
			page_buffer[ page_index++ ] = data[i];
		}
		else // Page is full...
		{
			// Program page into Flash.
			w25n01gv_write_enable();
			w25n01gv_quad_data_load( 0, 2048, page_buffer ); // Fill the entire internal buffer
			w25n01gv_program_execute( (write_origin + write_index)/PAGE_SIZE );
			while( w25n01gv_is_busy() )
				vTaskDelay( pdMS_TO_TICKS(50) );

			write_index += PAGE_SIZE;

			// If starting new block, erase it
			if ( ( (write_origin+write_index) % BLOCK_SIZE) == 0 )
			{
				w25n01gv_write_enable();
				w25n01gv_block_erase( (write_origin + write_index)/BLOCK_SIZE );
				while( w25n01gv_is_busy() )
					vTaskDelay( pdMS_TO_TICKS(50) );
			}

			// Restart filling page buffer
			page_index = 0;
			page_buffer[ page_index++ ] = data[i];

		}
	}
}


int image_ext_flash_close( void )
{

	uint8_t remain = page_index;

	// Complete the page with 0xFF
	if (page_index != 0)
		while( page_index < PAGE_SIZE )
			page_buffer[ page_index++ ] = 0xFF;

	// Program page into Flash.
	w25n01gv_write_enable();
	w25n01gv_quad_data_load( 0, 2048, page_buffer ); // Fill the entire internal buffer
	w25n01gv_program_execute( (write_origin + write_index)/PAGE_SIZE );
	while( w25n01gv_is_busy() )
		vTaskDelay( pdMS_TO_TICKS(50) );

	return 0;
}

void image_ext_flash_read( int block_number, uint32_t init_byte, uint32_t len, uint8_t* data)
{
	uint32_t read_index = 0;
	uint32_t bytes_to_read;
	const uint32_t addr = ( block_number * BLOCK_SIZE ) + init_byte;

	while( read_index < len )
	{
		// Load page
		w25n01gv_page_data_read( (addr+read_index)/PAGE_SIZE );
		while( w25n01gv_is_busy() )
			vTaskDelay( pdMS_TO_TICKS(50) );

		bytes_to_read = PAGE_SIZE - ((addr+read_index)%PAGE_SIZE);
		if( (read_index + bytes_to_read) >= len )
			bytes_to_read = len - read_index;

		w25n01gv_fast_read_quad( (addr+read_index)%PAGE_SIZE, bytes_to_read, &data[read_index]);
		read_index += bytes_to_read;
	}
}

uint32_t image_ext_flash_caclulate_CRC( uint32_t addr, uint32_t len )
{
	uint8_t data[8] =  { 1, 2, 3, 4, 5, 6, 7, 8};
	uint32_t crc = ~HAL_CRC_Calculate(&hcrc, data, 4);
	crc = ~HAL_CRC_Accumulate(&hcrc, &data[4], 4);
	mt_printf("CRC: %x", crc);
}
