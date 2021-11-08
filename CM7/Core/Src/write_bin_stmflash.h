#ifndef WRITE_BIN_STMFLASH_H
#define WRITE_BIN_STMFLASH_H

#include "main.h"



void bin_stmflash_open( int sector_number );

int bin_stmflash_write( uint8_t* data, int len );

int bin_stmflash_close( void );

int bin_stmflash_validate( int sector_number, uint32_t* crc32 );

#endif // WRITE_BIN_STMFLASH_H
