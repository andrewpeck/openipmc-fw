

/*
 * This module implements a mechanism to write the firmware binary into the
 * internal STM32 Flash like a file. A small header is here used to keep info
 * about the written content.
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
 *
 * A 512 bytes header is reserved at the beginning, followed by the actual firmware data.
 * The reason behind a so large header is the Vector Table address, which must be multiple
 * of 0x200.
 */


/*
 * HEADER STRUCTURE
 *
 * byte#  |   Size  |  Description
 *        |
 *    0   | 4 bytes |  BINARY_IS_PRESENT_0 (Magic Number)
 *    4   | 4 bytes |  BINARY_IS_PRESENT_1 (Magic Number)
 *    8   | 4 bytes |  Payload size (size of the binary)
 *   12   | 4 bytes |  CRC32 of payload
 */


#include "main.h"
#include "write_bin_stmflash.h"


// Flash characteristics
#define FLASH_ORIGIN  0x8000000  // First address of Flash in absolute address
#define SECTOR_SIZE   0x20000    // Size of erasable sector
#define PROG_SIZE     32         // Flash is programmed in groups of 256 bits (32 bytes)

#define HEADER_SIZE 512          // Size of the header

// Magic number to confirm the presence of binary
#define BINARY_IS_PRESENT_0  0xF11B9859
#define BINARY_IS_PRESENT_1  0xD4FA9FB4

extern CRC_HandleTypeDef hcrc;

static uint32_t write_origin;
static uint32_t write_index;

// Buffer to hold the 32 bytes to be programmed at once
static uint8_t prog_buffer[PROG_SIZE];
static uint8_t prog_buffer_index;


static void stm32_flash_erase_sector( uint32_t sector_start_addr );


void bin_stmflash_open( int sector_number )
{
	write_origin      = FLASH_ORIGIN + (sector_number * SECTOR_SIZE);
	write_index       = HEADER_SIZE;  // Reserve the Header
	prog_buffer_index = 0;

	stm32_flash_erase_sector( write_origin );
}



int bin_stmflash_write( uint8_t* data, int len )
{

	for( int i=0; i<len; i++)
	{
		if( prog_buffer_index <  PROG_SIZE)
		{
			prog_buffer[ prog_buffer_index++ ] = data[i];
		}
		else
		{
			// If starting new sector, erase it
			if ( ( (write_origin+write_index-FLASH_ORIGIN) % SECTOR_SIZE) == 0 )
			{
				stm32_flash_erase_sector( write_origin+write_index );
			}

			// Program prog_buffer into Flash
			HAL_FLASH_Unlock();
			__HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR | FLASH_FLAG_PGSERR);
			HAL_FLASH_Program(FLASH_TYPEPROGRAM_FLASHWORD, write_origin + write_index, (uint32_t)prog_buffer);
			HAL_FLASH_Lock();

			write_index += PROG_SIZE;

			// Restart filling prog_buffer
			prog_buffer_index = 0;
			prog_buffer[ prog_buffer_index++ ] = data[i];

			// Progress
			if( (write_index % (4*1024) ) == 0 )
				mt_printf(".");
		}

	}


}


int bin_stmflash_close( void )
{

	uint8_t remain = prog_buffer_index;

	// Takes care for padding if needed.
	if (prog_buffer_index != 0)
	{
		while( prog_buffer_index < PROG_SIZE )
			prog_buffer[ prog_buffer_index++ ] = 0xFF;

		// If starting new sector, erase it
		if ( ( (write_origin+write_index-FLASH_ORIGIN) % SECTOR_SIZE) == 0 )
		{
			stm32_flash_erase_sector( write_origin+write_index );
		}

		// Program prog_buffer into Flash
		HAL_FLASH_Unlock();
		__HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR | FLASH_FLAG_PGSERR);
		HAL_FLASH_Program(FLASH_TYPEPROGRAM_FLASHWORD, write_origin + write_index, (uint32_t)prog_buffer);
		HAL_FLASH_Lock();
	}

	// Write the header
	for( int i=0; i<32; i++) { prog_buffer[i] = 0xFF; }
	*((uint32_t*)(&prog_buffer[0])) = BINARY_IS_PRESENT_0;
	*((uint32_t*)(&prog_buffer[4])) = BINARY_IS_PRESENT_1;
	*((uint32_t*)(&prog_buffer[8])) = write_index + remain - HEADER_SIZE; // Payload size
	*((uint32_t*)(&prog_buffer[12])) = ~HAL_CRC_Calculate( &hcrc, (uint32_t)(write_origin+HEADER_SIZE), write_index + remain - HEADER_SIZE );; // CRC32

	HAL_FLASH_Unlock();
	__HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR | FLASH_FLAG_PGSERR);
	HAL_FLASH_Program(FLASH_TYPEPROGRAM_FLASHWORD, write_origin, (uint32_t)prog_buffer);
	HAL_FLASH_Lock();

	mt_printf("\r\n %d bytes written into Flash\r\n", write_index + remain - HEADER_SIZE);
	return write_index + remain - HEADER_SIZE;
}


int bin_stmflash_validate( int sector_number, uint32_t* crc32 )
{
	const uint32_t origin = FLASH_ORIGIN + (sector_number * SECTOR_SIZE);
	const uint32_t payload_origin = origin + HEADER_SIZE;
	const uint32_t payload_size   = *((uint32_t*)(origin+8));

	*crc32 = 0;

	// Check magic numbers
	if( ( *((uint32_t*)(origin+0)) != BINARY_IS_PRESENT_0 ) ||
	    ( *((uint32_t*)(origin+4)) != BINARY_IS_PRESENT_1 )	   )
	{
		return 0; // Binary is not valid
	}

	uint32_t calculated_crc = ~HAL_CRC_Calculate( &hcrc, (uint32_t)payload_origin, payload_size );

	// Compare calculated CRC with the header CRC
	if( *((uint32_t*)(origin+12)) != calculated_crc )
		return 0; // Binary is not valid

	*crc32 = calculated_crc;

	return 1; // binary is valid!
}



static void stm32_flash_erase_sector( uint32_t sector_start_addr )
{
	uint32_t sector_number = (sector_start_addr-FLASH_ORIGIN)/SECTOR_SIZE;

	HAL_FLASH_Unlock();

	__HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR | FLASH_FLAG_PGSERR);
	FLASH_Erase_Sector(sector_number, FLASH_BANK_BOTH, FLASH_VOLTAGE_RANGE_1);

	HAL_FLASH_Lock();

}

