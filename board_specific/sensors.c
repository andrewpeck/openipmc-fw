
#include <stdint.h>

#include "sdr_definitions.h"


/*
 * Sensor reading callbacks
 *
 * These callbacks are assigned to each sensor when the sensors are created.
 * They are defined at the end of this file
 *
 * Sensors are create in the create_board_specific_sensors() hook below.
 *
 *
 */
sensor_reading_status_t sensor_reading_fpga_temp(sensor_reading_t* sensor_reading);
sensor_reading_status_t sensor_reading_air_temp(sensor_reading_t* sensor_reading);
sensor_reading_status_t sensor_reading_vcc_out(sensor_reading_t* sensor_reading);



/*
 * Hook for creating board specific sensors
 *
 * This function is called during the OpenIPMC initialization and is dedicated
 * for creating the board-specific sensors.
 */
void create_board_specific_sensors(void)
{

	uint8_t threshold_list[6];

	// Dummy sensor for FPGA temperature.
	threshold_list[0] = 0;    // Lower Non Recoverable NOT USED
	threshold_list[1] = 0;    // Lower Critical        NOT USED
	threshold_list[2] = 0;    // Lower Non Critical    NOT USED
	threshold_list[3] = 65;   // Upper Non Critical    65°C
	threshold_list[4] = 75;   // Upper Critical        75°C
	threshold_list[5] = 100;  // Upper Non Recoverable 100°C
	create_generic_analog_sensor_1( TEMPERATURE,
	                                DEGREES_C,
	                                1,            // y = 1*x + 0  (temperature in °C is identical to it raw value)
	                                0,
	                                0,
	                                0,
	                                UPPER_NON_CRITICAL | UPPER_CRITICAL | UPPER_NON_RECOVERABLE,
	                                threshold_list,
	                                "FPGA TEMP",
	                                &sensor_reading_fpga_temp );


	// Dummy sensor for Air temperature.
	threshold_list[0] = 0;    // Lower Non Recoverable  NOT USED
	threshold_list[1] = 0;    // Lower Critical         NOT USED
	threshold_list[2] = 0;    // Lower Non Critical     NOT USED
	threshold_list[3] = 100;  // Upper Non Critical     30°C  (see conversion below)
	threshold_list[4] = 120;  // Upper Critical         40°C
	threshold_list[5] = 0;    // Upper Non Recoverable  NOT USED
	create_generic_analog_sensor_1( TEMPERATURE,
	                                DEGREES_C,
	                                5,            // y = (0.5*x - 20) = (5*x - 200)*0.1
	                                -200,
	                                0,
	                                -1,
	                                UPPER_NON_CRITICAL | UPPER_CRITICAL,
	                                threshold_list,
	                                "AIR TEMP",
	                                &sensor_reading_air_temp );


	// Dummy sensor for Voltage.
	threshold_list[0] = 0;    // Lower Non Recoverable  NOT USED
	threshold_list[1] = 0;    // Lower Critical         NOT USED
	threshold_list[2] = 0;    // Lower Non Critical     NOT USED
	threshold_list[3] = 0;    // Upper Non Critical     NOT USED
	threshold_list[4] = 0;    // Upper Critical         NOT USED
	threshold_list[5] = 0;    // Upper Non Recoverable  NOT USED
	create_generic_analog_sensor_1( VOLTAGE,
	                                VOLTS,
	                                1,            // y = 0.1*x = (1*x + 0)*0.1
	                                0,
	                                0,
	                                -1,
	                                0, // No thresholds
	                                threshold_list,
	                                "12V_RAIL",
	                                &sensor_reading_vcc_out );
}


/*
 * Sensor Reading functions
 */
sensor_reading_status_t sensor_reading_fpga_temp(sensor_reading_t* sensor_reading)
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

	return SENSOR_READING_OK;
}


sensor_reading_status_t sensor_reading_air_temp(sensor_reading_t* sensor_reading)
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

	return SENSOR_READING_OK;
}

sensor_reading_status_t sensor_reading_vcc_out(sensor_reading_t* sensor_reading)
{

	// This sensor uses y = 0.1*x for conversion
	sensor_reading->raw_value = 124; // 12.4V

	sensor_reading->present_state = 0; // No thresholds supported by this sensor

	return SENSOR_READING_OK;
}
