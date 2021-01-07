#ifndef AMC_GPIOS_H
#define AMC_GPIOS_H


typedef enum
{
    OUT = 0,
     IN = 1   // Default. All pins as input after initialization.
} amc_dir_t;


typedef enum
{
    OFF = 0, // Default. All pin are floating after initialization.
     ON = 1
} amc_pullup_t;


void         amc_gpios_initialize_expanders( void );
void         amc_gpios_set_pin_direction( uint8_t amc_pin, amc_dir_t dir );
amc_dir_t    amc_gpios_get_pin_direction( uint8_t amc_pin );
void         amc_gpios_write_pin( uint8_t amc_pin, uint8_t pin_value );
uint8_t      amc_gpios_read_pin( uint8_t amc_pin );
void         amc_gpios_set_pin_pullup( uint8_t amc_pin, amc_pullup_t pullup_state );
amc_pullup_t amc_gpios_get_pin_pullup( uint8_t amc_pin );


#endif
