

#include "main.h"



/*
 * Define how to control SPI_LE for CDCE62002 by using GPIO.
 */
#define EXPANDER_NSS_set_low()  HAL_GPIO_WritePin( EXPANDER_NSS_GPIO_Port, EXPANDER_NSS_Pin,  GPIO_PIN_RESET);
#define EXPANDER_NSS_set_high() HAL_GPIO_WritePin( EXPANDER_NSS_GPIO_Port, EXPANDER_NSS_Pin,  GPIO_PIN_SET  );

#define TIMEOUT 100

extern SPI_HandleTypeDef hspi4;
#define hspi_amc hspi4


void amc_gpios_initialize_expanders(void)
{
	uint8_t command[4];

	// Reset expanders
	// TODO

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
