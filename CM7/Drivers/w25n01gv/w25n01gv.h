#ifndef W25N01GV_H
#define W25N01GV_H

#include <stdint.h>
#include <stdbool.h>


void    w25n01gv_device_reset(void);
uint8_t w25n01gv_get_status_reg(int reg);
void    w25n01gv_set_status_reg(int reg, uint8_t value);
bool    w25n01gv_is_busy(void);
void    w25n01gv_write_enable(void);

void w25n01gv_data_load( uint16_t column, uint32_t len, uint8_t* data );
void w25n01gv_quad_data_load( uint16_t column, uint32_t len, uint8_t* data );
void w25n01gv_program_execute( uint16_t page );

void w25n01gv_page_data_read( uint16_t page );
void w25n01gv_read( uint16_t column, uint32_t len, uint8_t* data);
void w25n01gv_fast_read_quad( uint16_t column, uint32_t len, uint8_t* data);

void w25n01gv_block_erase(uint16_t block);



#endif
