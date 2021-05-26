
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

#ifndef SENSE_I2C_H
#define SENSE_I2C_H

#include "main.h"

/*
 * Initialization function
 */
void sense_i2c_init( void );


/*
 * This functions encapsulates the STM32 HAL I2C
 *
 * Internally, it uses the I2C3 port (peripheral handle is being omitted in this prototypes for this reason).
 *
 * It also implements mutex for multithread compatibility.
 *
 * Original driver header: stm32h7xx_hal_i2c.h
 */
HAL_StatusTypeDef sense_i2c_transmit( uint16_t DevAddress, uint8_t *pData, uint16_t Size, uint32_t Timeout );
HAL_StatusTypeDef sense_i2c_receive( uint16_t DevAddress, uint8_t *pData, uint16_t Size, uint32_t Timeout );



#endif // SENSE_I2C_H
