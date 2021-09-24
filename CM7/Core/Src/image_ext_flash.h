#ifndef IMAGE_EXT_FLASH_H
#define IMAGE_EXT_FLASH_H

void image_ext_flash_open( int block_number );
void image_ext_flash_write( uint8_t* data, int len );
int  image_ext_flash_close( void );

void image_ext_flash_read( int block_number, uint32_t init_byte, uint32_t len, uint8_t* data);

#endif
