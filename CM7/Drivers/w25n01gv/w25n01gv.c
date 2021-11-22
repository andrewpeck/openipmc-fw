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
 * This file implements a driver for W25N01GV Winbond Flash Memory over STM32 QSPI
 *
 * This driver is also compatible with the W74M01GV Winbond Flash Memory.
 *
 */

#include "main.h"
#include "w25n01gv.h"



#define INIT_CMD_WITH_DEFAULTS(COMMAND)                              \
	{                                                                \
        COMMAND.Instruction = 0;                                     \
        COMMAND.InstructionMode = QSPI_INSTRUCTION_1_LINE;           \
		                                                             \
		COMMAND.Address = 0;                                         \
		COMMAND.AddressMode = QSPI_ADDRESS_NONE;                     \
		COMMAND.AddressSize = QSPI_ADDRESS_8_BITS;                   \
                                                                     \
        COMMAND.AlternateBytes = 0;                                  \
		COMMAND.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;       \
		COMMAND.AlternateBytesSize = QSPI_ALTERNATE_BYTES_8_BITS;    \
                                                                     \
        COMMAND.DummyCycles = 0;                                     \
		                                                             \
		COMMAND.DataMode = QSPI_DATA_NONE;                           \
        COMMAND.NbData = 0;                                          \
		                                                             \
		COMMAND.DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY;        \
		COMMAND.DdrMode = QSPI_DDR_MODE_DISABLE;                     \
		COMMAND.SIOOMode = QSPI_SIOO_INST_EVERY_CMD;                 \
	}


// Handler to the QSPI, which must be already configured
extern QSPI_HandleTypeDef hqspi;


void w25n01gv_device_reset(void)
{

	QSPI_CommandTypeDef cmd;

	INIT_CMD_WITH_DEFAULTS(cmd);

	cmd.Instruction = 0xFF; // Device RESET

	HAL_QSPI_Command  ( &hqspi, &cmd,  100 );
}


uint8_t w25n01gv_get_status_reg(int reg)
{

	QSPI_CommandTypeDef cmd;
	uint8_t data;

	INIT_CMD_WITH_DEFAULTS(cmd);

	cmd.Instruction = 0x05; // Read Status Register
	cmd.AddressMode = QSPI_ADDRESS_1_LINE;
	cmd.Address     = reg;
	cmd.DataMode    = QSPI_DATA_1_LINE;
	cmd.NbData      = 1;

	HAL_QSPI_Command ( &hqspi, &cmd,  100 );
	HAL_QSPI_Receive ( &hqspi, &data, 100 );

	return data;
}


void w25n01gv_set_status_reg(int reg, uint8_t value)
{

	QSPI_CommandTypeDef cmd;

	INIT_CMD_WITH_DEFAULTS(cmd);

	cmd.Instruction = 0x1F; // Write Status Register
	cmd.AddressMode = QSPI_ADDRESS_1_LINE;
	cmd.Address     = reg;
	cmd.DataMode    = QSPI_DATA_1_LINE;
	cmd.NbData      = 1;

	HAL_QSPI_Command ( &hqspi, &cmd,   100 );
	HAL_QSPI_Transmit( &hqspi, &value, 100 );
}

_Bool w25n01gv_is_busy(void)
{

	QSPI_CommandTypeDef cmd;
	uint8_t data;

	INIT_CMD_WITH_DEFAULTS(cmd);

	cmd.Instruction = 0x05; // Read Status Register
	cmd.AddressMode = QSPI_ADDRESS_1_LINE;
	cmd.Address     = 0xC0; // Status Register-3
	cmd.DataMode    = QSPI_DATA_1_LINE;
	cmd.NbData      = 1;

	HAL_QSPI_Command ( &hqspi, &cmd,  100 );
	HAL_QSPI_Receive ( &hqspi, &data, 100 );

	return (data & 0x01);
}

void w25n01gv_write_enable(void)
{

	QSPI_CommandTypeDef cmd;

	INIT_CMD_WITH_DEFAULTS(cmd);

	cmd.Instruction = 0x06; // Write Enable

	HAL_QSPI_Command  ( &hqspi, &cmd, 100 );
}



/*
 * Program Data Load
 *
 * Load data into the Data Buffer
 *
 *   column : The number of the page column to be loaded
 *      len : Number of bytes to be loaded
 *     data : Buffer containing the data to be loaded
 */
void w25n01gv_data_load( uint16_t column, uint32_t len, uint8_t* data )
{
	QSPI_CommandTypeDef cmd;

	INIT_CMD_WITH_DEFAULTS(cmd);

	cmd.Instruction = 0x02; // Program Data Load
	cmd.AddressMode = QSPI_ADDRESS_1_LINE;
	cmd.AddressSize = QSPI_ADDRESS_16_BITS;
	cmd.Address     = column;
	cmd.DataMode    = QSPI_DATA_1_LINE;
	cmd.NbData      = len;

	HAL_QSPI_Command  ( &hqspi, &cmd, 100 );
	HAL_QSPI_Transmit ( &hqspi, data, 100 );
}




/*
 * Quad Program Data Load
 *
 * Load data into the Data Buffer
 *
 *   column : The number of the page column to be loaded
 *      len : Number of bytes to be loaded
 *     data : Buffer containing the data to be loaded
 */
void w25n01gv_quad_data_load( uint16_t column, uint32_t len, uint8_t* data )
{
	QSPI_CommandTypeDef cmd;

	INIT_CMD_WITH_DEFAULTS(cmd);

	cmd.Instruction = 0x32; // Quad Program Data Load
	cmd.AddressMode = QSPI_ADDRESS_1_LINE;
	cmd.AddressSize = QSPI_ADDRESS_16_BITS;
	cmd.Address     = column;
	cmd.DataMode    = QSPI_DATA_4_LINES;
	cmd.NbData      = len;

	HAL_QSPI_Command  ( &hqspi, &cmd, 100 );
	HAL_QSPI_Transmit ( &hqspi, data, 100 );
}




/*
 * Program Execute
 *
 * Program the content of the Data Buffer into a page.
 *
 *   page : The number of the memory page to be programmed, form 0 to 65535 (16 bits)
 */
void w25n01gv_program_execute( uint16_t page )
{
	QSPI_CommandTypeDef cmd;

	INIT_CMD_WITH_DEFAULTS(cmd);

	cmd.Instruction = 0x10; // Program Execute
	cmd.AddressMode = QSPI_ADDRESS_1_LINE;
	cmd.AddressSize = QSPI_ADDRESS_24_BITS; // 1 dummy byte + 2 address bytes
	cmd.Address     = page;

	HAL_QSPI_Command  ( &hqspi, &cmd,  100 );
}





/*
 * Page Data Read
 *
 * Load the content of a page into the Data Buffer
 *
 *   page : The number of the memory page to be read, form 0 to 65535 (16 bits)
 */
void w25n01gv_page_data_read( uint16_t page )
{
	QSPI_CommandTypeDef cmd;

	INIT_CMD_WITH_DEFAULTS(cmd);

	cmd.Instruction = 0x13; // Page Data Read
	cmd.AddressMode = QSPI_ADDRESS_1_LINE;
	cmd.AddressSize = QSPI_ADDRESS_24_BITS; // 1 dummy byte + 2 address bytes
	cmd.Address     = page;

	HAL_QSPI_Command  ( &hqspi, &cmd,  100 );
}


/*
 * Read
 *
 * Read data from the Data Buffer
 *
 *   column : The number of the page column to be read
 *      len : Number of bytes to be read
 *     data : Buffer to receive the bytes
 */
void w25n01gv_read( uint16_t column, uint32_t len, uint8_t* data )
{
	QSPI_CommandTypeDef cmd;

	INIT_CMD_WITH_DEFAULTS(cmd);

	cmd.Instruction = 0x03; // Read
	cmd.AddressMode = QSPI_ADDRESS_1_LINE;
	cmd.AddressSize = QSPI_ADDRESS_16_BITS;
	cmd.Address     = column;
	cmd.DummyCycles = 8;
	cmd.DataMode    = QSPI_DATA_1_LINE;
	cmd.NbData      = len;

	HAL_QSPI_Command ( &hqspi, &cmd, 100 );
	HAL_QSPI_Receive ( &hqspi, data,100 );
}



/*
 * Fast Read Quad Output
 *
 * Read data from the Data Buffer
 *
 *   column : The number of the page column to be read
 *      len : Number of bytes to be read
 */
void w25n01gv_fast_read_quad( uint16_t column, uint32_t len, uint8_t* data )
{
	QSPI_CommandTypeDef cmd;

	INIT_CMD_WITH_DEFAULTS(cmd);

	cmd.Instruction = 0x6B; // Fast Read Quad Output
	cmd.AddressMode = QSPI_ADDRESS_1_LINE;
	cmd.AddressSize = QSPI_ADDRESS_16_BITS;
	cmd.Address     = column;
	cmd.DummyCycles = 8;
	cmd.DataMode    = QSPI_DATA_4_LINES;
	cmd.NbData      = len;

	HAL_QSPI_Command ( &hqspi, &cmd, 100 );
	HAL_QSPI_Receive ( &hqspi, data, 100 );
}



/*
 * Erase one block
 *
 *   block : The number of the memory block to be erased, form 0 to 1023 (10 bits)
 */
void w25n01gv_block_erase( uint16_t block )
{
	QSPI_CommandTypeDef cmd;

	INIT_CMD_WITH_DEFAULTS(cmd);

	uint16_t page = (block & 0x3FF) <<6;

	cmd.Instruction = 0xD8; // Block Erase
	cmd.AddressMode = QSPI_ADDRESS_1_LINE;
	cmd.AddressSize = QSPI_ADDRESS_24_BITS; // 1 dummy byte + 2 address bytes
	cmd.Address     = page;

	HAL_QSPI_Command  ( &hqspi, &cmd,  100 );
}



/*
 * QSPI test
 *
 * Load and read back the Data Buffer to check the device responsivity via QSPI
 *
 * return: 0 if ok; 0 if fail.
 */
//uint8_t flash_qspi_test( void )
//{
//	flash_device_reset();
//
//	flash_set_status_reg(0xA0, 0x00);
//
//	for( int j = 0; j<2048; j++ )
//	 flash_buffer_test[j] = (j & 0xFF);
//
//	flash_write_enable();
//	flash_quad_data_load( 0, 2048, flash_buffer_test );
//
//	for( int j = 0; j<2048; j++ )
//		  flash_buffer_test[j] = 0;
//
//	flash_fast_read_quad( 0, 2048, flash_buffer_test );
//
//	for( int j = 0; j<2048; j++ )
//		if(flash_buffer_test[j] != (j & 0xFF))
//			return 1; // FAIL
//
//	return 0;
//}




