/*
 * This file contains code which are used by OpenIPMC to perform operations over
 * the ATCA board "payload".
 */

#include "cmsis_os.h"

//OpenIPMC includes
#include "dimm_gpios.h"
#include "power_manager.h"
#include "sdr_definitions.h"



uint8_t test_temp_value = 50;



/*
 * Switch Power Level
 *
 * This functions is called by OpenIPMC when a Power Level change is requested by
 * Shelf Manager
 */
void ipmc_pwr_switch_power_level_on_payload( uint8_t new_power_level )
{

	/*
	 * Do whatever is needed to set to the requested power level on the payload
	 */

	// Typical operation over 12V_EN signal on DIMM
	if(new_power_level != 0)
		EN_12V_SET_STATE(SET);
	else
		EN_12V_SET_STATE(RESET);

	return;
}

/*
 * Payload Cold Reset
 *
 * This functions is called by OpenIPMC when a Cold Reset command is received
 * from Shelf Manager
 */
void payload_cold_reset (void)
{

	// Typical operation over PAYLOAD_RESET signal on DIMM
	PAYLOAD_RESET_SET_STATE(RESET);
	osDelay(10); // Holds PAYLOAD_RESET Low for 10ms
	PAYLOAD_RESET_SET_STATE(SET);

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
