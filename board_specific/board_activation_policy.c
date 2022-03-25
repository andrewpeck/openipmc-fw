
#include <dimm_gpios.h>
#include <stdint.h>

#include "power_manager.h"


/*
 * Power Initialization.
 *
 * This function is called during the OpenIPMC initialization and is dedicated
 * for configuring the board-specific power levels.
 */
void config_power_levels(void)
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
  //pwr_envelope.power_draw[20] = 220;

  ipmc_pwr_setup_power_envelope(pwr_envelope);  // Copy the envelope to the power manager

  // Here must be informed the Power Level desired by the payload. It must be a valid index of the power_draw array (1 up to num_of_levels).
  ipmc_pwr_set_desired_power_level(2); // Power Level 2 means 100 Watts, following what is specified in the power_draw array above.
}
