#include <stdint.h>
#include <stddef.h>

/* FreeRTOS includes. */
//#include "FreeRTOS.h"

#include "dimm_gpios.h"

//OpenIPMC includes
#include "sdr_manager.h"
#include "power_manager.h"
#include "fru_inventory_manager.h"
#include "sdr_definitions.h"
//#include "ipmc_ios.h"
#include "sensors_templates.h"

void power_initialization(void);

uint8_t test_temp_value = 50;

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

	board_info->field_area = create_board_info_field (board_info->field_ptr,// MAXIMUM SIZE OF EACH STRING IS 64 CHARACTERS / FIELD AREA IS MULTIPLE OF 8 BYTES
														13017600, 			// Manufacturing Date/Time (in minutes from January 1th 1996)
														"SPRACE-KIT", 		// Board Manufacturer,
														"OpenIPMC-HW",		// Board Product Name,
														"000000-0001",	    // Serial Number,
														"XXXXXXX1",			// Part Number,
														"01");// FRU File ID

	create_fru_inventory (	NULL, // Internal Use Field NOT USED
   							NULL, // Chassis Info Field NOT USED
							board_info,
							NULL, // Product Info Field NOT USED
							NULL);// Multi Record Field NOT USED

	vPortFree (board_info->field_ptr);
	vPortFree (board_info);


	/*
	 *  Create Sensors
	 */
    init_sdr_repository();

    create_management_controller_sdr ("IPMC Zync US+");

    create_hot_swap_carrier_sensor ("Hot Swap Carrier");

    // PRARAMETERS for create_simple_temperature_sensor function:
    // base_unit (°C, °F...),
    // linearization (linear, log10...),
    // m (Y = mX + b),
    // b,
    // accuracy,
    // upper non recoverable threshold, INSERT RAW DATA
    // upper critical threshold,        INSERT RAW DATA
    // upper non critical threshold,    INSERT RAW DATA
    // id string
    create_simple_temperature_sensor (DEGREES_C, LINEAR, 1, 0, 1, 85, 75, 65, "FPGA TEMP", &sensor_reading_fpga_temp);
    create_simple_temperature_sensor (DEGREES_C, LINEAR, 1, 0, 1, 65, 60, 55, "AIR TEMP", &sensor_reading_air_temp);

    // PRARAMETERS for create_voltage_out_sensor function:
    // linearization (linear, log10...),
    // m (Y = mX + b),
    // b,
    // accuracy,
    // r_exp, R = Y*10^r_exp (result exponent - SIGNED 4 bits, MSB of field); C0h = -4
    // upper non critical threshold, //  INSERT RAW DATA (must be converted using Y = mX +b and r_exp)
    // lower non critical threshold, //  INSERT RAW DATA (must be converted using Y = mX +b and r_exp)
    // id string
    create_simple_voltage_out_sensor(LINEAR, 157, 0, 1, 0xc0, 0x46, 0x39, "VCC1V0 VOUT", &sensor_reading_vcc_out);


}

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

void payload_cold_reset (void)
{
    // user defined for reseting the payload
    ipmc_ios_printf("\nCOLD RESET SUCCESSFUL!\r\n");

}

uint8_t get_fru_control_capabilities (void)
{
    uint8_t const capabilities = 0; // WARM_RESET_SUPPORTED | GRACEFUL_REBOOT_SUPPORTED | DIAGNOSTIC_INTERRUPT_SUPPORTED
    return capabilities;
}


void power_initialization(void)
{

	power_envelope_t pwr_envelope;

	pwr_envelope.num_of_levels =  2; // Max of 20 beyond the 0-th value
	pwr_envelope.multiplier    = 10; // Global multiplier in tenths of Watt

	// REMINDER: PICMG spec **requires** the maximum power envelope values in this array to increase monotonically!
	pwr_envelope.power_draw[ 0] = 0;   // Power Level 0: RESERVED and always means 0 Watt (payload off)
	pwr_envelope.power_draw[ 1] = 20;  // Power Level 1: 20 Watts ( power_draw[1] * multiplier * 0.1W = 20 * 10 * 0.1 W = 20W )
	pwr_envelope.power_draw[ 2] = 100; // Power Level 2: 100 Watts
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



void ipmc_pwr_switch_power_level_on_payload(uint8_t new_power_level)
{

	ipmc_ios_printf("Change Power Level from %d to %d\r\n", ipc_pwr_get_current_power_level(), new_power_level);
	/*
	 * Do whatever is needed to set to the requested power level on the payload
	 */

	if(new_power_level != 0)
		EN_12V_SET_STATE(SET);
	else
		EN_12V_SET_STATE(RESET);

	return;
}
