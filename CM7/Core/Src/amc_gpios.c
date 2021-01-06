

#include "main.h"

#include "amc_gpios.h"

struct
{
	uint8_t device;
	uint8_t port; // 0 = portA; 1 = portB
	uint8_t pin;

} pin_map[90] = { {0, 0, 0}, {0, 0, 1}, {0, 0, 2}, {0, 0, 3}, {0, 0, 4}, {0, 0, 5}, {0, 0, 6}, {0, 0, 7}, {0, 1, 0}, {0, 1, 1}, // AMC0_IO_n
                  {1, 1, 7}, {1, 1, 6}, {1, 1, 5}, {1, 1, 4}, {1, 1, 3}, {1, 1, 2}, {1, 1, 1}, {1, 1, 0}, {1, 0, 7}, {1, 0, 6}, // AMC1_IO_n
                  {0, 1, 2}, {0, 1, 3}, {0, 1, 4}, {0, 1, 5}, {0, 1, 6}, {0, 1, 7}, {2, 0, 0}, {2, 0, 1}, {2, 0, 2}, {2, 0, 3},
                  {1, 0, 5}, {1, 0, 4}, {1, 0, 3}, {1, 0, 2}, {1, 0, 1}, {1, 0, 0}, {3, 1, 7}, {3, 1, 6}, {3, 1, 5}, {3, 1, 4},
                  {2, 0, 4}, {2, 0, 5}, {2, 0, 6}, {2, 0, 7}, {2, 1, 0}, {2, 1, 1}, {2, 1, 2}, {2, 1, 3}, {2, 1, 4}, {2, 1, 5},
                  {3, 1, 3}, {3, 1, 2}, {3, 1, 1}, {3, 1, 0}, {3, 0, 7}, {3, 0, 6}, {3, 0, 5}, {3, 0, 4}, {3, 0, 3}, {3, 0, 2},
                  {2, 1, 6}, {2, 1, 7}, {4, 0, 0}, {4, 0, 1}, {4, 0, 2}, {4, 0, 3}, {4, 0, 4}, {4, 0, 5}, {4, 0, 6}, {4, 0, 7},
                  {3, 0, 1}, {3, 0, 0}, {5, 1, 7}, {5, 1, 6}, {5, 1, 5}, {5, 1, 4}, {5, 1, 3}, {5, 1, 2}, {5, 1, 1}, {5, 1, 0},
                  {4, 1, 0}, {4, 1, 1}, {4, 1, 2}, {4, 1, 3}, {4, 1, 4}, {4, 1, 5}, {4, 1, 6}, {4, 1, 7}, {5, 0, 7}, {5, 0, 6}
                                                                                                                               };

/*
 * Define how to control ExPANDER_NSS and EXPANDER_RST.
 */
#define EXPANDER_NSS_set_low()  HAL_GPIO_WritePin( EXPANDER_NSS_GPIO_Port, EXPANDER_NSS_Pin,  GPIO_PIN_RESET);
#define EXPANDER_NSS_set_high() HAL_GPIO_WritePin( EXPANDER_NSS_GPIO_Port, EXPANDER_NSS_Pin,  GPIO_PIN_SET  );
#define EXPANDER_RST_set_low()  HAL_GPIO_WritePin( EXPANDER_RST_GPIO_Port, EXPANDER_RST_Pin,  GPIO_PIN_RESET);
#define EXPANDER_RST_set_high() HAL_GPIO_WritePin( EXPANDER_RST_GPIO_Port, EXPANDER_RST_Pin,  GPIO_PIN_SET  );

#define TIMEOUT 100

extern SPI_HandleTypeDef hspi4;
#define hspi_amc hspi4

void    amc_gpios_set_expander_register(uint8_t device_addr, uint8_t reg_addr, uint8_t value);
uint8_t amc_gpios_get_expander_register(uint8_t device_addr, uint8_t reg_addr);
void    set_expander_register_bit(uint8_t device_addr, uint8_t reg_addr, uint8_t bit_posic, uint8_t bit_value);




void amc_gpios_initialize_expanders( void )
{
	uint8_t command[4];

	// Reset expanders
	EXPANDER_RST_set_low();
	HAL_Delay(10);
	EXPANDER_RST_set_high();

	// Configure expanders in "Address Mode".
	// (Since "Address Mode" is not enable after reset, this operation has
	// a broadcast effect)
	command[0] = 0x40;
	command[1] = 0x0A; // IOCON register
	command[2] = 0x08; // Set IOCON.HAEN bit
	EXPANDER_NSS_set_low();
	HAL_SPI_Transmit(&hspi_amc, command, 3, TIMEOUT);
	EXPANDER_NSS_set_high();

	// The previous command must be repeated with A2 set to '1'.
	// Even if a device is not in "Address Mode", it is still sensitive to the pin A2.
	// This is a BUG reported in the MCP23S17 Rev. A  Silicon Errata (DS80311A)
	command[0] = 0x48; //Set A2 to '1'
	EXPANDER_NSS_set_low();
	HAL_SPI_Transmit(&hspi_amc, command, 3, TIMEOUT);
	EXPANDER_NSS_set_high();
}

void amc_gpios_set_expander_register( uint8_t device_addr, uint8_t reg_addr, uint8_t value )
{
	uint8_t command[3];

	command[0] = 0x40 | ((device_addr & 0x07) << 1);
	command[1] = reg_addr & 0x0F;
	command[2] = value;

	EXPANDER_NSS_set_low();
	HAL_SPI_Transmit(&hspi_amc, command, 3, TIMEOUT);
	EXPANDER_NSS_set_high();
}

uint8_t amc_gpios_get_expander_register( uint8_t device_addr, uint8_t reg_addr )
{
	uint8_t command[3];
	uint8_t data[3];

	command[0] = 0x41 | ((device_addr & 0x07) << 1);
	command[1] = reg_addr & 0x0F;
	command[2] = 0;

	EXPANDER_NSS_set_low();
	HAL_SPI_TransmitReceive(&hspi_amc, command, data, 3, TIMEOUT);
	EXPANDER_NSS_set_high();

	return data[2];
}

void set_expander_register_bit( uint8_t device_addr, uint8_t reg_addr, uint8_t bit_posic, uint8_t bit_value )
{
	uint8_t command[3];
	uint8_t data[3];

	command[0] = 0x41 | ( device_addr<<1 );
	command[1] = reg_addr;
	command[2] = 0;             // Transmit Zero while receive data from device

	// Get current value
	EXPANDER_NSS_set_low();
	HAL_SPI_TransmitReceive(&hspi_amc, command, data, 3, TIMEOUT);
	EXPANDER_NSS_set_high();

	command[0] &=~0x01; // Clear R/W bit
	command[2]  = data[2] &~(0x01<<bit_posic); // The target bit is cleared
	if( bit_value != 0 )
		command[2] |= ( 0x01<<bit_posic );

	// Write new value
	EXPANDER_NSS_set_low();
	HAL_SPI_Transmit(&hspi_amc, command, 3, TIMEOUT);
	EXPANDER_NSS_set_high();
}


uint8_t get_expander_register_bit( uint8_t device_addr, uint8_t reg_addr, uint8_t bit_posic )
{
	uint8_t command[3];
	uint8_t data[3];

	command[0] = 0x41 | ( device_addr<<1 );
	command[1] = reg_addr;
	command[2] = 0;             // Transmit Zero while receive data from device

	// Get current value
	EXPANDER_NSS_set_low();
	HAL_SPI_TransmitReceive(&hspi_amc, command, data, 3, TIMEOUT);
	EXPANDER_NSS_set_high();

	if( ( data[2] & (1<<bit_posic) ) != 0 )
		return 1;
	else
		return 0;
}


void amc_gpios_set_pin_direction( uint8_t amc_pin, amc_pin_dir_t dir )
{
	uint8_t reg_addr;

	// Protection against bad value
	if(amc_pin >= 90) return;

	// Calculate register addr IODIRA or IODIRB. Base addr: 0x00
	reg_addr = 0x00 + pin_map[amc_pin].port;

	set_expander_register_bit( pin_map[amc_pin].device, reg_addr, pin_map[amc_pin].pin, (uint8_t)dir );
}

void amc_gpios_write_pin( uint8_t amc_pin, uint8_t pin_value )
{
	uint8_t reg_addr;

	// Protection against bad value
	if(amc_pin >= 90) return;

	// Calculate register addr GPIOA or GPIOB. Base addr: 0x12
	reg_addr = 0x12 + pin_map[amc_pin].port;

	set_expander_register_bit( pin_map[amc_pin].device, reg_addr, pin_map[amc_pin].pin, pin_value );
}

uint8_t amc_gpios_read_pin( uint8_t amc_pin )
{
	uint8_t reg_addr;

	// Protection against bad value
	if(amc_pin >= 90) return 0;

	// Calculate register addr GPIOA or GPIOB. Base addr: 0x12
	reg_addr = 0x12 + pin_map[amc_pin].port;

	return get_expander_register_bit( pin_map[amc_pin].device, reg_addr, pin_map[amc_pin].pin );
}
