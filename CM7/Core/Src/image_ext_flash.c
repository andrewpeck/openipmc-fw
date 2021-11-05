
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
 * on the external flash memory (Winbond W25N01GV), and others related tools.
 */

#include "main.h"

#include "FreeRTOS.h"
#include "task.h"

#include "w25n01gv.h"
#include "fw_metadata.h"

// Flash characteristics
#define BLOCK_SIZE   131072   // Size of erasable block: 128KB or 64 pages = 131072 bytes
#define PAGE_SIZE      2048   // Flash is programmed in pages of 2048 bytes

// Info for firmware backup
#define OPENIPMC_CM7_RUN_ADDR 0x08000000
#define OPENIPMC_CM7_BACKUP_BLOCK 0 // Backup is made into the beginning of external flash

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

			// If starting new block, erase it
			if ( ( (write_origin+write_index) % BLOCK_SIZE) == 0 )
			{
				w25n01gv_write_enable();
				w25n01gv_block_erase( (write_origin + write_index)/BLOCK_SIZE );
				while( w25n01gv_is_busy() )
					vTaskDelay( pdMS_TO_TICKS(10) );
			}

			// Program page into Flash.
			w25n01gv_write_enable();
			w25n01gv_quad_data_load( 0, 2048, page_buffer ); // Fill the entire internal buffer
			w25n01gv_program_execute( (write_origin + write_index)/PAGE_SIZE );
			while( w25n01gv_is_busy() )
				vTaskDelay( pdMS_TO_TICKS(10) );

			write_index += PAGE_SIZE;

			// Restart filling page buffer
			page_index = 0;
			page_buffer[ page_index++ ] = data[i];

		}
	}
}


int image_ext_flash_close( void )
{

	uint8_t remain = page_index;

	// If an incomplete page is pending...
	if (page_index != 0)
	{
		while( page_index < PAGE_SIZE )
			page_buffer[ page_index++ ] = 0xFF; // Fill the rest with 0xFF

		// If starting new block, erase it
		if ( ( (write_origin+write_index) % BLOCK_SIZE) == 0 )
		{
			w25n01gv_write_enable();
			w25n01gv_block_erase( (write_origin + write_index)/BLOCK_SIZE );
			while( w25n01gv_is_busy() )
				vTaskDelay( pdMS_TO_TICKS(10) );
		}

		// Program page into Flash.
		w25n01gv_write_enable();
		w25n01gv_quad_data_load( 0, 2048, page_buffer ); // Fill the entire internal buffer
		w25n01gv_program_execute( (write_origin + write_index)/PAGE_SIZE );
		while( w25n01gv_is_busy() )
		vTaskDelay( pdMS_TO_TICKS(10) );
	}
	
	return 0;
}

/*
 * Destroy the image by deleting the first block
 */
void image_ext_flash_delete( int block_number )
{
	// Erase first used block
	w25n01gv_write_enable();
	w25n01gv_block_erase( block_number );
	while( w25n01gv_is_busy() )
		vTaskDelay( pdMS_TO_TICKS(10) );
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
			vTaskDelay( pdMS_TO_TICKS(10) );

		bytes_to_read = PAGE_SIZE - ((addr+read_index)%PAGE_SIZE);
		if( (read_index + bytes_to_read) >= len )
			bytes_to_read = len - read_index;

		w25n01gv_fast_read_quad( (addr+read_index)%PAGE_SIZE, bytes_to_read, &data[read_index]);
		read_index += bytes_to_read;
	}
}

bool image_ext_flash_CRC_is_valid( int block_number )
{
	metadata_fields_v0_t metadata_fields;

	uint32_t read_index = 0;
	uint32_t bytes_to_read;

	uint32_t crc_from_tail;
	uint32_t calculated_crc;

	// Read metadata
	image_ext_flash_read( block_number, FW_METADATA_ADDR, sizeof(metadata_fields), (uint8_t*)&metadata_fields);

	// Check presence word
	if( metadata_fields.presence_word != FW_METADATA_PRESENCE_WORD )
		return false;

	// Check sum
	uint32_t sum = 0;
	for( int i=0; i<(sizeof(metadata_fields)/sizeof(uint32_t)); i++ )
		sum += ((uint32_t*)(&metadata_fields))[i];
	if( sum != 0 )
		return false;

	// Get CRC32 present at the end of image
	image_ext_flash_read( block_number, metadata_fields.image_size, sizeof(crc_from_tail), (uint8_t*)&crc_from_tail);

	calculated_crc = ~HAL_CRC_Calculate(&hcrc, (uint32_t*)page_buffer, 0); // Reset CRC32 calculator

	// Feed CRC32 calculator
	while( read_index < metadata_fields.image_size )
	{
		bytes_to_read = metadata_fields.image_size - read_index;
		if( bytes_to_read > PAGE_SIZE )
			bytes_to_read = PAGE_SIZE;

		image_ext_flash_read( block_number, read_index, bytes_to_read, page_buffer);
		calculated_crc = ~HAL_CRC_Accumulate(&hcrc, (uint32_t*)page_buffer, bytes_to_read);
		read_index += bytes_to_read;
	}

	if( calculated_crc == crc_from_tail )
		return true;
	else
		return false;
}

/*
 * Perform the backup of CM7 firmware present on the STM32 internal flash (ADDR: 0x08000000)
 *
 * Firmware is copied to the beginning of external flash present on DIMM
 *
 * return FALSE if backup fails
 */
bool image_ext_flash_openipmc_CM7_backup( void )
{
	metadata_fields_v0_t* fw_metadata = (metadata_fields_v0_t*)(OPENIPMC_CM7_RUN_ADDR + FW_METADATA_ADDR);

	// Check metadata checksum
	uint32_t sum = 0;
	for( int i=0; i<(sizeof(metadata_fields_v0_t)/sizeof(uint32_t)); i++ )
		sum += ((uint32_t*)fw_metadata)[i];
	if( sum != 0 )
		return false;

	// Calculate CRC32 of the image present on the internal flash
	uint32_t calculated_crc = ~HAL_CRC_Calculate(&hcrc, (uint32_t*)OPENIPMC_CM7_RUN_ADDR, fw_metadata->image_size);

	// Copy image to external flash (and add the CRC)
	image_ext_flash_open( OPENIPMC_CM7_BACKUP_BLOCK );
	image_ext_flash_write( (uint8_t*)OPENIPMC_CM7_RUN_ADDR, fw_metadata->image_size );
	image_ext_flash_write( (uint8_t*)&calculated_crc, sizeof(uint32_t) );
	image_ext_flash_close();

	//Check copy
	if( image_ext_flash_CRC_is_valid( OPENIPMC_CM7_BACKUP_BLOCK ) )
		return true;
	else
		return false;
}


