#ifndef IMAGE_EXT_FLASH_H
#define IMAGE_EXT_FLASH_H

#include <stdint.h>
#include <stdbool.h>

void image_ext_flash_open( int block_number );
void image_ext_flash_write( uint8_t* data, int len );
int  image_ext_flash_close( void );

void image_ext_flash_read( int block_number, uint32_t init_byte, uint32_t len, uint8_t* data);

bool image_ext_flash_CRC_is_valid( int block_number );

int  image_ext_flash_openipmc_CM7_backup( void );

#endif
