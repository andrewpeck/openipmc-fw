
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

#include "main.h"
#include "cmsis_os.h"
#include "semphr.h"

#include "sense_i2c.h"

static SemaphoreHandle_t sense_i2c_mutex = NULL;
static StaticSemaphore_t sense_i2c_mutex_buffer;

extern I2C_HandleTypeDef hi2c3;
#define hi2c_sense hi2c3

#define ENABLE_I2C_PERIPH() \
	{                                                                     \
		hi2c_sense.Instance->CR1 |= I2C_CR1_PE;                           \
		while( !(hi2c_sense.Instance->CR1 & I2C_CR1_PE) ) { asm("nop"); } \
		__HAL_I2C_ENABLE_IT( &hi2c_sense, I2C_IT_ERRI );                  \
	}

#define DISABLE_I2C_PERIPH() \
	{                                                                    \
		hi2c_sense.Instance->CR1 &=~ I2C_CR1_PE;                         \
		while( (hi2c_sense.Instance->CR1 & I2C_CR1_PE) ) { asm("nop"); } \
	}



void sense_i2c_init( void )
{
	sense_i2c_mutex = xSemaphoreCreateMutexStatic( &sense_i2c_mutex_buffer );
}


uint32_t sense_i2c_transmit( uint16_t DevAddress, uint8_t *pData, uint16_t Size, uint32_t Timeout )
{
	uint32_t i2c_error_code;

	while (sense_i2c_mutex == NULL)  { asm("nop"); }
	xSemaphoreTake( sense_i2c_mutex, portMAX_DELAY );

	ENABLE_I2C_PERIPH();
	HAL_I2C_Master_Transmit( &hi2c_sense, DevAddress, pData, Size, Timeout );
	i2c_error_code = hi2c_sense.ErrorCode;
	DISABLE_I2C_PERIPH();

	xSemaphoreGive( sense_i2c_mutex );

	return i2c_error_code;
}



uint32_t sense_i2c_receive( uint16_t DevAddress, uint8_t *pData, uint16_t Size, uint32_t Timeout )
{
	uint32_t i2c_error_code;

	while (sense_i2c_mutex == NULL)  { asm("nop"); }
	xSemaphoreTake( sense_i2c_mutex, portMAX_DELAY );

	ENABLE_I2C_PERIPH();
	HAL_I2C_Master_Receive( &hi2c_sense, DevAddress, pData, Size, Timeout );
	i2c_error_code = hi2c_sense.ErrorCode;
	DISABLE_I2C_PERIPH();

	xSemaphoreGive( sense_i2c_mutex );

	return i2c_error_code;
}




