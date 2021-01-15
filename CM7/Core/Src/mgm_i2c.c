
#include "main.h"
#include "cmsis_os.h"
#include "semphr.h"

#include "mgm_i2c.h"

static SemaphoreHandle_t mgm_i2c_mutex = NULL;
static StaticSemaphore_t mgm_i2c_mutex_buffer;

extern I2C_HandleTypeDef hi2c4;

void mgm_i2c_init( void )
{
	mgm_i2c_mutex = xSemaphoreCreateMutexStatic( &mgm_i2c_mutex_buffer );
}


HAL_StatusTypeDef mgm_i2c_transmit( uint16_t DevAddress, uint8_t *pData, uint16_t Size, uint32_t Timeout )
{
	HAL_StatusTypeDef status;

	while (mgm_i2c_mutex == NULL)  { asm("nop"); }
	xSemaphoreTake( mgm_i2c_mutex, portMAX_DELAY );
	status = HAL_I2C_Master_Transmit( &hi2c4, DevAddress, pData, Size, Timeout );
	xSemaphoreGive( mgm_i2c_mutex );

	return status;
}



HAL_StatusTypeDef mgm_i2c_receive( uint16_t DevAddress, uint8_t *pData, uint16_t Size, uint32_t Timeout )
{
	HAL_StatusTypeDef status;

	while (mgm_i2c_mutex == NULL)  { asm("nop"); }
	xSemaphoreTake( mgm_i2c_mutex, portMAX_DELAY );
	status = HAL_I2C_Master_Receive( &hi2c4, DevAddress, pData, Size, Timeout );
	xSemaphoreGive( mgm_i2c_mutex );

	return status;
}




