#ifndef AMC_GPIOS_H
#define AMC_GPIOS_H


typedef enum
{
    OUT = 0,
     IN = 1
} amc_pin_dir_t;


void    amc_gpios_initialize_expanders( void );
void    amc_gpios_set_pin_direction( uint8_t amc_pin, amc_pin_dir_t dir );
void    amc_gpios_write_pin( uint8_t amc_pin, uint8_t pin_value );
uint8_t amc_gpios_read_pin( uint8_t amc_pin );


#endif
