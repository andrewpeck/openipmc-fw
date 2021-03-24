#include <stdint.h>
#include <stddef.h>

#include <FreeRTOS.h>

//OpenIPMC includes
#include "sdr_manager.h"
#include "power_manager.h"
#include "fru_inventory_manager.h"
#include "sdr_definitions.h"
#include "sensors_templates.h"
#include "apollo/pim400.h"

static void power_initialization(void);

/*
 * Sensor reading functions
 *
 * These functions tell OpenIPMC how to get the sensor reading.
 *
 * Defined on openipmc_brd_ctrls.c file
 */
void sensor_reading_fpga_temp(sensor_reading_t* sensor_reading);
void sensor_reading_air_temp(sensor_reading_t* sensor_reading);
void sensor_reading_vcc_out(sensor_reading_t* sensor_reading);


/*
 * This function is called by OpenIPMC during initialization process. The
 * following initializations are expected to be done by this function:
 *   - Initial power scheme
 *   - FRU Inventory
 *   - Sensor Records (if any)
 */
void ipmc_custom_initialization()
{

	/*
	 * Power Initialization.
	 *
	 * To simplify the example, the Power Initialization was wrapped in a function
	 * defined later in this file.
	 */
	power_initialization();

	/*
	 *  Create FRU Inventory
	 */
	fru_inventory_field_t *board_info   = pvPortMalloc(sizeof(fru_inventory_field_t)); // Inventory Fields must be allocated and filled before FRU Inventory is created
	board_info->field_ptr = pvPortMalloc(sizeof(uint8_t)*256);

	board_info->field_area = create_board_info_field( board_info->field_ptr,// MAXIMUM SIZE OF EACH STRING IS 64 CHARACTERS / FIELD AREA IS MULTIPLE OF 8 BYTES
	                                                  13017600,             // Manufacturing Date/Time (in minutes from January 1th 1996)
	                                                  "SPRACE-KIT",         // Board Manufacturer,
	                                                  "OpenIPMC-HW",        // Board Product Name,
	                                                  "000000-0001",        // Serial Number,
	                                                  "XXXXXXX1",           // Part Number,
	                                                  "01");                // FRU File ID

	create_fru_inventory ( NULL, // Internal Use Field NOT USED
	                       NULL, // Chassis Info Field NOT USED
	                       board_info,
	                       NULL, // Product Info Field NOT USED
	                       NULL);// Multi Record Field NOT USED

	vPortFree (board_info->field_ptr);
	vPortFree (board_info);


	/*
	 *  Create Sensors
	 */

	uint8_t threshold_list[6];

	init_sdr_repository();

	create_management_controller_sdr ("IPMC Zync US+");

	create_hot_swap_carrier_sensor ("Hot Swap Carrier");



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
	                                0,
	                                threshold_list,
	                                "12V_RAIL",
	                                &sensor_reading_vcc_out );

	#include "apollo/sensor_inits.h"

}



uint8_t get_fru_control_capabilities (void)
{
	uint8_t const capabilities = 0; // WARM_RESET_SUPPORTED | GRACEFUL_REBOOT_SUPPORTED | DIAGNOSTIC_INTERRUPT_SUPPORTED
	return capabilities;
}


static void power_initialization(void)
{

	power_envelope_t pwr_envelope;

	pwr_envelope.num_of_levels =  1; // Max of 20 beyond the 0-th value
	pwr_envelope.multiplier    = 10; // Global multiplier in tenths of Watt

	// REMINDER: PICMG spec **requires** the maximum power envelope values in this array to increase monotonically!
	pwr_envelope.power_draw[ 0] = 0;   // Power Level 0: RESERVED and always means 0 Watt (payload off)
	pwr_envelope.power_draw[ 1] = 100;  // Power Level 1: 20 Watts ( power_draw[1] * multiplier * 0.1W = 20 * 10 * 0.1 W = 20W )
	//pwr_envelope.power_draw[ 2] = 100; // Power Level 2: 100 Watts
	//pwr_envelope.power_draw[ 3] = 110;
	//pwr_envelope.power_draw[ 4] = 140;
	//pwr_envelope.power_draw[ 5] = 150;
	//pwr_envelope.power_draw[ 6] = 160;
	//pwr_envelope.power_draw[ 7] = 170;
	//pwr_envelope.power_draw[ 8] = 180;
	//pwr_envelope.power_draw[ 9] = 190;
	//pwr_envelope.power_draw[10] = 210;
	//pwr_envelope.power_draw[11] = 211;
	//pwr_envelope.power_draw[12] = 212;
	//pwr_envelope.power_draw[13] = 213;
	//pwr_envelope.power_draw[14] = 214;
	//pwr_envelope.power_draw[15] = 215;
	//pwr_envelope.power_draw[16] = 216;
	//pwr_envelope.power_draw[17] = 217;
	//pwr_envelope.power_draw[18] = 218;
	//pwr_envelope.power_draw[19] = 219;
	//pwr_envelope.power_draw[20] = 310;

	ipmc_pwr_setup_power_envelope(pwr_envelope);  // Copy the envelope to the power manager

	// Here must be informed the Power Level desired by the payload. It must be a valid index of the power_draw array (1 up to num_of_levels).
	ipmc_pwr_set_desired_power_level(2); // Power Level 2 means 100 Watts, following what is specified in the power_draw array above.
}
