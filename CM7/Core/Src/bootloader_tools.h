#ifndef AMC_GPIOS_H
#define AMC_GPIOS_H

#include <stdbool.h>

bool bootloader_is_present( uint8_t* major_version, uint8_t* minor_version, uint8_t aux_version[4] );
bool bootloader_is_active( void );

bool bootloader_enable( void );
bool bootloader_disable( void );

void bootloader_schedule_load( uint8_t boot_ctrl_load_mode );

#endif /* AMC_GPIOS_H */
