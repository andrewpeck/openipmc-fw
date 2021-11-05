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

#ifndef IMAGE_EXT_FLASH_H
#define IMAGE_EXT_FLASH_H

#include <stdint.h>
#include <stdbool.h>

void image_ext_flash_open( int block_number );
void image_ext_flash_write( uint8_t* data, int len );
int  image_ext_flash_close( void );
void image_ext_flash_delete( int block_number );

void image_ext_flash_read( int block_number, uint32_t init_byte, uint32_t len, uint8_t* data);

bool image_ext_flash_CRC_is_valid( int block_number );

int  image_ext_flash_openipmc_CM7_backup( void );

#endif
