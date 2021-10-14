

/*
 * This module implements a mechanism to write the firmware image into the
 * internal STM32 Flash like a file.
 *
 * It is based in the "write file" operations: OPEN, WRITE and CLOSE
 *
 * This module manages the erase and program operations according to the specific
 * requirements of this Flash memory
 *
 * This module is mainly intended to write firmware binary to the internal flash.
 * For this reason some simplification are considered:
 *   - The WRITE function will erase a new 128K sector if needed, with no care about data loss
 *   - Write origin must is aligned to the origin of a 128K sector
 */

#include <stdbool.h>

#include "main.h"

#include "fw_metadata.h"


// Flash characteristics
#define FLASH_ORIGIN  0x8000000  // First address of Flash in absolute address
#define SECTOR_SIZE   0x20000    // Size of erasable sector
#define PROG_SIZE     32         // Flash is programmed in groups of 256 bits (32 bytes)

static uint32_t write_origin;
static uint32_t write_index;

// Buffer to hold the 32 bytes to be programmed at once
static uint8_t prog_buffer[PROG_SIZE];
static uint8_t prog_buffer_index;

extern CRC_HandleTypeDef hcrc;

static void stm32_flash_erase_sector( uint32_t sector_start_addr );


void image_int_flash_open( int sector_number )
{
	write_origin      = FLASH_ORIGIN + (sector_number * SECTOR_SIZE);
	write_index       = 0;
	prog_buffer_index = 0;

	stm32_flash_erase_sector( write_origin );
}



int image_int_flash_write( uint8_t* data, int len )
{

	for( int i=0; i<len; i++)
	{
		if( prog_buffer_index <  PROG_SIZE)
		{
			prog_buffer[ prog_buffer_index++ ] = data[i];
		}
		else // Page is full...
		{
			// Program prog_buffer into Flash
			HAL_FLASH_Unlock();
			__HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR | FLASH_FLAG_PGSERR);
			HAL_FLASH_Program(FLASH_TYPEPROGRAM_FLASHWORD, write_origin + write_index, (uint32_t)prog_buffer);
			HAL_FLASH_Lock();

			write_index += PROG_SIZE;

			// If starting new sector, erase it
			if ( ( (write_origin+write_index-FLASH_ORIGIN) % SECTOR_SIZE) == 0 )
			{
				stm32_flash_erase_sector( write_origin+write_index );
			}

			// Restart filling prog_buffer
			prog_buffer_index = 0;
			prog_buffer[ prog_buffer_index++ ] = data[i];
		}

	}

	return 0;
}


int image_int_flash_close( void )
{

	uint8_t remain = prog_buffer_index;

	// Complete the page with 0xFF
	if (prog_buffer_index != 0)
		while( prog_buffer_index < PROG_SIZE )
			prog_buffer[ prog_buffer_index++ ] = 0xFF;

	// Program prog_buffer into Flash
	HAL_FLASH_Unlock();
	__HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR | FLASH_FLAG_PGSERR);
	HAL_FLASH_Program(FLASH_TYPEPROGRAM_FLASHWORD, write_origin + write_index, (uint32_t)prog_buffer);
	HAL_FLASH_Lock();

	return write_index + remain;
}

/*
 * Calculates the CRC of the image present into the internal flash and
 * compares it with the expected one
 */
bool image_int_flash_CRC_is_valid( int sector_number, uint32_t expected_crc32 )
{
	const uint32_t origin = FLASH_ORIGIN + (sector_number * SECTOR_SIZE);

	metadata_fields_v0_t* metadata = (metadata_fields_v0_t*)(origin + FW_METADATA_ADDR);

	// Check presence word
	if( metadata->presence_word != FW_METADATA_PRESENCE_WORD )
		return false;

	// Check sum
	uint32_t sum = 0;
	for( int i=0; i<(sizeof(metadata_fields_v0_t)/sizeof(uint32_t)); i++ )
		sum += ((uint32_t*)metadata)[i];
	if( sum != 0 )
		return false;

	uint32_t calculated_crc = ~HAL_CRC_Calculate( &hcrc, (uint32_t*)origin, metadata->image_size );

	mt_printf("Internal CRC: %X\r\n", calculated_crc);

	// Compare calculated CRC with the header CRC
	if( expected_crc32 != calculated_crc )
		return false; // Binary is not valid

	return true; // binary is valid!
}


static void stm32_flash_erase_sector( uint32_t sector_start_addr )
{
	uint32_t sector_number = (sector_start_addr-FLASH_ORIGIN)/SECTOR_SIZE;
	uint32_t bank_number;

	if( sector_number < 8 )
		bank_number = FLASH_BANK_1;
	else
		bank_number = FLASH_BANK_2;

	HAL_FLASH_Unlock();

	__HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR | FLASH_FLAG_PGSERR);
	FLASH_Erase_Sector(sector_number, bank_number, FLASH_VOLTAGE_RANGE_1);

	HAL_FLASH_Lock();

}

/*** TEST ****/
#include "image_ext_flash.h"
uint8_t upgrade_buffer[512];

void test_upgrade(void)
{
	int origin_block = 8;
	int destination_sector = 4;
	int segment = 512;

	metadata_fields_v0_t metadata_fields;

	uint32_t read_index = 0;
	uint32_t bytes_to_read;

	if( !image_ext_flash_CRC_is_valid( origin_block ) )
	{
		mt_printf("No valid image in ext flash\r\n");
		return;
	}

	// Read metadata
	image_ext_flash_read( origin_block, FW_METADATA_ADDR, sizeof(metadata_fields), (uint8_t*)&metadata_fields);

	// Write internal flash
	image_int_flash_open( destination_sector );
	while( read_index < metadata_fields.image_size )
	{
		bytes_to_read = metadata_fields.image_size - read_index;
		if( bytes_to_read > segment )
			bytes_to_read = segment;

		image_ext_flash_read( origin_block, read_index, bytes_to_read, upgrade_buffer);
		image_int_flash_write( upgrade_buffer, bytes_to_read);
		read_index += bytes_to_read;
	}
	image_int_flash_close();

	if( !image_int_flash_CRC_is_valid( destination_sector, 0xfb3b8939 ) )
	{
		mt_printf("No valid image in int flash\r\n");
		return;
	}

	mt_printf("Upgrade OK\r\n");
}



