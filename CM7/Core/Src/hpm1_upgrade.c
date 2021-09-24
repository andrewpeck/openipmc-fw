
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
 * This file implements the hardware specific operations required by HPM1 Upgrade
 * in OpenIPMC submodule
 */

#include <stdbool.h>
#include <string.h>
#include "main.h"

#include "FreeRTOS.h"
#include "task.h"

#include "hpm1_ctrls.h"
#include "w25n01gv.h"

extern CRC_HandleTypeDef hcrc;
uint32_t crc32_from_agent;
uint32_t crc32_local;
bool extracted_crc;

void image_ext_flash_open( int block_number );
void image_ext_flash_write( uint8_t* data, int len );
int  image_ext_flash_close( void );
void image_ext_flash_read( uint32_t addr, uint32_t len, uint8_t* data);


/*
 * Function called during the OpenIPMC HAL initializations.
 * It prepares the HPM1 Upgrade functionality by setting attributes
 */
void hpm1_init(void)
{

	hpm1_global_capabilities = 0;
	hpm1_timeouts.inaccessibility_timeout = 1;
	hpm1_timeouts.rollback_timeout = 1;
	hpm1_timeouts.selftest_timeout = 1;
	hpm1_timeouts.upgrade_timeout = 1;

	hpm1_add_component( 0 );

	hpm1_component_properties[0]->payload_cold_reset_is_required   = false;
	hpm1_component_properties[0]->deffered_activation_is_supported = false;
	hpm1_component_properties[0]->firmware_comparison_is_supported = false;
	hpm1_component_properties[0]->rollback_is_supported = true;
	hpm1_component_properties[0]->backup_cmd_is_required = true;

	hpm1_component_properties[0]->current_firmware_revision_major = 1;
	hpm1_component_properties[0]->current_firmware_revision_minor = 0x23;
	hpm1_component_properties[0]->current_firmware_revision_aux   = 0x12abcdef;

	strncpy( hpm1_component_properties[0]->description_string, "CM7_fw", 12 );

	hpm1_component_properties[0]->rollback_firmware_revision_major = 0x5;
	hpm1_component_properties[0]->rollback_firmware_revision_minor = 0x67;
	hpm1_component_properties[0]->rollback_firmware_revision_aux   = 0x12000000;

	hpm1_component_properties[0]->backup_is_available = false;
}


void hpm1_cmd_initiate_backup_cb( uint8_t component_mask )
{
	mt_printf("BACKUP 0x%x\r\n", component_mask);
}

void hpm1_cmd_initiate_prepare_cb( uint8_t component_mask )
{
	mt_printf("PREPARE 0x%x\r\n", component_mask);
}

void hpm1_cmd_initiate_upload_for_upgrade_cb( uint8_t component_number )
{
	mt_printf("INIT UPLOAD UPGRADE %d\r\n", component_number);

	image_ext_flash_open( 0 );
	extracted_crc = false;
}

void hpm1_cmd_initiate_upload_for_compare_cb( uint8_t component_number )
{
	mt_printf("INIT UPLOAD COMPARE %d\r\n", component_number);

}

void hpm1_cmd_upload_cb( uint8_t component_number, uint8_t* block_data, uint8_t block_size  )
{
	mt_printf("UPLOAD IMAGE %d\r\n", component_number);
	for (int i = 0; i<block_size; i++) mt_printf("%d ", block_data[i]);
	mt_printf("\r\n");
	//vTaskDelay( pdMS_TO_TICKS(100) );

	if(!extracted_crc)
	{
		crc32_from_agent = ( (uint32_t)block_data[3]<<24 ) + ( (uint32_t)block_data[2]<<16 ) + ( (uint32_t)block_data[1]<<8 ) + ( (uint32_t)block_data[0]<<0 );
		mt_printf("CRC from agent: %x\r\n", crc32_from_agent);
		extracted_crc = true;

		image_ext_flash_write( &block_data[4], block_size-4 );
		HAL_CRC_Calculate(&hcrc, &block_data[4], block_size-4);
	}
	else
	{
		image_ext_flash_write( block_data, block_size );
		crc32_local = ~HAL_CRC_Accumulate( &hcrc, block_data, block_size );
	}
}

void hpm1_cmd_upload_finish_cb( uint8_t component_number  )
{
	mt_printf("FINISH UPLOAD %d\r\n", component_number);
	image_ext_flash_close();
	mt_printf("CRC local: %x\r\n", crc32_local);
}

void hpm1_cmd_activate_cb( void )
{
	mt_printf("ACTIVATE \r\n");
}





// FLASH *******************************************


// Flash characteristics
#define BLOCK_SIZE   131072   // Size of erasable block: 128KB or 64 pages = 131072 bytes
#define PAGE_SIZE      2048   // Flash is programmed in pages of 2048 bytes

static uint32_t write_origin;
static uint32_t write_index;

// Buffer in RAM for 2048 bytes flash page
// TODO move this buffer to another bigger and slower region
static uint8_t page_buffer[PAGE_SIZE];
static int     page_index;


#define HEADER_SIZE 512          // Size of the header containing image info


void image_ext_flash_open( int block_number )
{
	write_origin  = block_number * BLOCK_SIZE;
	write_index   = 0;
	page_index    = HEADER_SIZE; // Reserve the Header

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

	// Takes care for padding if needed.
	if (page_index != 0)
		while( page_index < PAGE_SIZE )
			page_buffer[ page_index++ ] = 0xFF;

	// Program page into Flash.
	w25n01gv_write_enable();
	w25n01gv_quad_data_load( 0, 2048, page_buffer ); // Fill the entire internal buffer
	w25n01gv_program_execute( (write_origin + write_index)/PAGE_SIZE );
	while( w25n01gv_is_busy() )
		vTaskDelay( pdMS_TO_TICKS(50) );



	// Write the header
	w25n01gv_page_data_read( write_origin/PAGE_SIZE ); // Read the initial page
	while( w25n01gv_is_busy() )
		vTaskDelay( pdMS_TO_TICKS(50) );

//	*((uint32_t*)(&prog_buffer[0])) = BINARY_IS_PRESENT_0;
//	*((uint32_t*)(&prog_buffer[4])) = BINARY_IS_PRESENT_1;
//	*((uint32_t*)(&prog_buffer[8])) = write_index + remain - HEADER_SIZE; // Payload size
//	*((uint32_t*)(&prog_buffer[12])) = ~HAL_CRC_Calculate( &hcrc, (uint32_t*)(write_origin+HEADER_SIZE), write_index + remain - HEADER_SIZE );; // CRC32
//
//	HAL_FLASH_Unlock();
//	__HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR | FLASH_FLAG_PGSERR);
//	HAL_FLASH_Program(FLASH_TYPEPROGRAM_FLASHWORD, write_origin, (uint32_t)prog_buffer);
//	HAL_FLASH_Lock();
//
//	mt_printf("\r\n %d bytes written into Flash\r\n", write_index + remain - HEADER_SIZE);
//	return write_index + remain - HEADER_SIZE;
}

void image_ext_flash_read( uint32_t addr, uint32_t len, uint8_t* data)
{
	uint32_t read_index = 0;
	uint32_t bytes_to_read;

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
