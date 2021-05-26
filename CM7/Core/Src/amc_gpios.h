
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

#ifndef AMC_GPIOS_H
#define AMC_GPIOS_H

#include "semphr.h"

// AMC_IO pin labeling
#define AMC0_IO_0   0
#define AMC0_IO_1   1
#define AMC0_IO_2   2
#define AMC0_IO_3   3
#define AMC0_IO_4   4
#define AMC0_IO_5   5
#define AMC0_IO_6   6
#define AMC0_IO_7   7
#define AMC0_IO_8   8
#define AMC0_IO_9   9
#define AMC1_IO_0   10
#define AMC1_IO_1   11
#define AMC1_IO_2   12
#define AMC1_IO_3   13
#define AMC1_IO_4   14
#define AMC1_IO_5   15
#define AMC1_IO_6   16
#define AMC1_IO_7   17
#define AMC1_IO_8   18
#define AMC1_IO_9   19
#define AMC2_IO_0   20
#define AMC2_IO_1   21
#define AMC2_IO_2   22
#define AMC2_IO_3   23
#define AMC2_IO_4   24
#define AMC2_IO_5   25
#define AMC2_IO_6   26
#define AMC2_IO_7   27
#define AMC2_IO_8   28
#define AMC2_IO_9   29
#define AMC3_IO_0   30
#define AMC3_IO_1   31
#define AMC3_IO_2   32
#define AMC3_IO_3   33
#define AMC3_IO_4   34
#define AMC3_IO_5   35
#define AMC3_IO_6   36
#define AMC3_IO_7   37
#define AMC3_IO_8   38
#define AMC3_IO_9   39
#define AMC4_IO_0   40
#define AMC4_IO_1   41
#define AMC4_IO_2   42
#define AMC4_IO_3   43
#define AMC4_IO_4   44
#define AMC4_IO_5   45
#define AMC4_IO_6   46
#define AMC4_IO_7   47
#define AMC4_IO_8   48
#define AMC4_IO_9   49
#define AMC5_IO_0   50
#define AMC5_IO_1   51
#define AMC5_IO_2   52
#define AMC5_IO_3   53
#define AMC5_IO_4   54
#define AMC5_IO_5   55
#define AMC5_IO_6   56
#define AMC5_IO_7   57
#define AMC5_IO_8   58
#define AMC5_IO_9   59
#define AMC6_IO_0   60
#define AMC6_IO_1   61
#define AMC6_IO_2   62
#define AMC6_IO_3   63
#define AMC6_IO_4   64
#define AMC6_IO_5   65
#define AMC6_IO_6   66
#define AMC6_IO_7   67
#define AMC6_IO_8   68
#define AMC6_IO_9   69
#define AMC7_IO_0   70
#define AMC7_IO_1   71
#define AMC7_IO_2   72
#define AMC7_IO_3   73
#define AMC7_IO_4   74
#define AMC7_IO_5   75
#define AMC7_IO_6   76
#define AMC7_IO_7   77
#define AMC7_IO_8   78
#define AMC7_IO_9   79
#define AMC8_IO_0   80
#define AMC8_IO_1   81
#define AMC8_IO_2   82
#define AMC8_IO_3   83
#define AMC8_IO_4   84
#define AMC8_IO_5   85
#define AMC8_IO_6   86
#define AMC8_IO_7   87
#define AMC8_IO_8   88
#define AMC8_IO_9   89



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

typedef enum
{
	AMC_INT_OFF,
	AMC_INT_BOTH_EDGES,
	//AMC_INT_RISING_EDGE,  To be developed
	//AMC_INT_FALLING_EDGE  To be developed
} amc_int_mode_t;

typedef enum
{
	AMC_INT_STATUS_NONE,
	AMC_INT_STATUS_FALLED,
	AMC_INT_STATUS_RISED
} amc_int_status_t;


extern osThreadId_t amc_gpios_pin_interrupt_task_handle;


/*
 * Initialization.
 */
void amc_gpios_init( void );


/*
 * User interface
 *
 * These functions are used to control the AMC_IOs. The driver employs mutex
 * to allow them multi-thread compatibility.
 *
 * IMPORTANT: Don't call these functions from ISRs context. The driver was not
 *            designed for such level of concurrency.
 *
 *   amc_pin: an integer from 0 to 89 representing all the AMC_IO pins. See
 *            the aliases defined previously in this file.
 */
void         amc_gpios_set_pin_direction( uint8_t amc_pin, amc_dir_t dir );
amc_dir_t    amc_gpios_get_pin_direction( uint8_t amc_pin );
void         amc_gpios_write_pin( uint8_t amc_pin, uint8_t pin_value );
uint8_t      amc_gpios_read_pin( uint8_t amc_pin );
void         amc_gpios_set_pin_pullup( uint8_t amc_pin, amc_pullup_t pullup_state );
amc_pullup_t amc_gpios_get_pin_pullup( uint8_t amc_pin );
void         amc_gpios_set_pin_interrupt( uint8_t amc_pin, amc_int_mode_t mode );


/*
 *  Callback to attend interrupts generated by the the AMC_IOs
 *
 *  This callback is called from a FreeRTOS task responsible for processing
 *  the interrupts from the MCP23S17
 */
void amc_gpios_pin_interrupt_callback( amc_int_status_t* interrupt_status );

// Interrupt handler for SPI complete transaction event.
void amc_gpios_spi_interrupt( void );

#endif
