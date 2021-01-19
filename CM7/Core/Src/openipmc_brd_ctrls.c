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

	sensor_reading->raw_value = test_temp_value;

	if((test_temp_value >= 65) && (test_temp_value < 75))
		sensor_reading->present_state = (1<<3);
	else if(test_temp_value >= 75)
		sensor_reading->present_state = (1<<4);
	else
		sensor_reading->present_state = 0;
}


void sensor_reading_air_temp(sensor_reading_t* sensor_reading)
{

	sensor_reading->raw_value = 32;

	sensor_reading->present_state = 0;

}

void sensor_reading_vcc_out(sensor_reading_t* sensor_reading)
{
	sensor_reading->raw_value = 22;

	sensor_reading->present_state = 0;
}
