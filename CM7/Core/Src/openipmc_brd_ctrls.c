/*
 * This file contains code which are used by OpenIPMC to perform operations over
 * the ATCA board "payload".
 */

#include "cmsis_os.h"
#include <stdbool.h>

//OpenIPMC includes
#include "dimm_gpios.h"
#include "power_manager.h"
#include "sdr_definitions.h"


#include "apollo/apollo.h"

struct
{
	
	_Bool   status;
	uint8_t current_power_level;
	uint8_t new_power_level;
	
} benchtop_mode = {false, 0, 0};


static void board_specific_activation_control( uint8_t current_power_level, uint8_t new_power_level );









/*
 * Switch Power Level
 *
 * This functions is called by OpenIPMC when a Power Level change is requested by
 * Shelf Manager
 */
void ipmc_pwr_switch_power_level_on_payload( uint8_t new_power_level )
{
	// Benchtop mode prevents actions from Shelf Manager
	if( benchtop_mode.status == true )
		return;
	
	uint8_t current_power_level = ipc_pwr_get_current_power_level();
	board_specific_activation_control( current_power_level, new_power_level );
}


/*
 * Board Specific routine for changing the Power Level
 * 
 * This function needs to be customized according to the board characteristic.
 */
void board_specific_activation_control( uint8_t current_power_level, uint8_t new_power_level )
{
	
	/*
	 * For customization, 'current_power_level' and 'new_power_level' can be used to improve
	 * any the transition between power levels.
	 */

	
	// DEACTIVATION
	if( new_power_level == 0)
	{
		
		// Customize DEACTIVATION process
		apollo_powerdown_sequence();
	}
	// ACTIVATION
	else {
		// Customize ACTIVATION process
		apollo_powerup_sequence();
	}
	return;
}



/*
 * Force a new power level in benchtop mode
 * 
 * In this mode, Shelf Manager looses the ability to change the Power Level
 */
void set_benchtop_payload_power_level( uint8_t new_power_level )
{
	benchtop_mode.status == true;
	
	board_specific_activation_control( benchtop_mode.current_power_level, new_power_level );
	
	benchtop_mode.current_power_level = new_power_level;
}



/*
 * Payload Cold Reset
 *
 * This functions is called by OpenIPMC when a Cold Reset command is received
 * from Shelf Manager
 */
void payload_cold_reset (void)
{
	PAYLOAD_RESET_SET_STATE(RESET);
	apollo_powerdown_sequence();

	PAYLOAD_RESET_SET_STATE(SET);
	apollo_powerup_sequence();
}


/*
 * Sensor Reading functions
 */
void sensor_reading_fpga_temp(sensor_reading_t* sensor_reading)
{

	// This sensor uses y = 1*x + 0 for conversion

	uint8_t raw_temp = 53; // 53°C

	// Fill the raw temp field
	sensor_reading->raw_value = raw_temp;

	// Fill the threshold flag field
	sensor_reading->present_state = 0;
	if(raw_temp > 65)
		sensor_reading->present_state |= UPPER_NON_CRITICAL;
	if(raw_temp > 75)
		sensor_reading->present_state |= UPPER_CRITICAL;
	if(raw_temp > 100)
		sensor_reading->present_state |= UPPER_NON_RECOVERABLE;
}


void sensor_reading_air_temp(sensor_reading_t* sensor_reading)
{

	// This sensor uses y = (0.5*x - 20) for conversion

	uint8_t raw_temp = 97; // 28.5°C

	// Fill the raw temp field
	sensor_reading->raw_value = raw_temp;

	// Fill the threshold flag field
	sensor_reading->present_state = 0;
	if(raw_temp > 100) // 30°C
		sensor_reading->present_state |= UPPER_NON_CRITICAL;
	if(raw_temp > 120) // 40°C
		sensor_reading->present_state |= UPPER_CRITICAL;

}

void sensor_reading_vcc_out(sensor_reading_t* sensor_reading)
{
	
	// This sensor uses y = 0.1*x for conversion
	sensor_reading->raw_value = 124; // 12.4V

	sensor_reading->present_state = 0; // No thresholds supported by this sensor
}
