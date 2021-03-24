#ifndef __APOLLO_SENSOR_INITS_H_
#define __APOLLO_SENSOR_INITS_H_

#include "sensor_helper.h"
#include "pim400.h"

//------------------------------------------------------------------------------
// PIM400
//------------------------------------------------------------------------------

// temperature -----------------------------------------------------------------

static const linear_sensor_constants_t pim400_temp =
{
  .sensor_type=TEMPERATURE,
  .unit_type=DEGREES_C,
  .lower_nonrecoverable=0,
  .lower_noncritical=0,
  .lower_critical=0,
  .upper_noncritical=(int) (50+75)/1.961,
  .upper_critical=(int) (50+80)/1.961,
  .upper_nonrecoverable=0,
  .m=1961,
  .b=-50,
  .e=3
};

create_linear_sensor (pim400_temp, "PIM400 Temperature", &sensor_reading_temp_pim400);

// current ---------------------------------------------------------------------

static const linear_sensor_constants_t pim400_iout =
{
  .sensor_type=CURRENT,
  .unit_type=AMPERES,
  .lower_nonrecoverable=0,
  .lower_noncritical=0,
  .lower_critical=0,
  .upper_noncritical=0,
  .upper_critical=0,
  .upper_nonrecoverable=0,
  .m=94,
  .b=0,
  .e=3
};

create_linear_sensor (pim400_iout, "PIM400 Current", &sensor_reading_iout_pim400);

// PIM400 48V input sensors (A and B)
//
static const linear_sensor_constants_t pim400_voltage =
{
  .sensor_type=VOLTAGE,
  .unit_type=VOLTS,
  .lower_nonrecoverable=0,
  .lower_noncritical=0,
  .lower_critical=0,
  .upper_noncritical=0,
  .upper_critical=0,
  .upper_nonrecoverable=0,
  .m=325,
  .b=0,
  .e=3
};

create_linear_sensor (pim400_voltage, "PIM400 -48V_A", &sensor_reading_voltage_a_pim400);
create_linear_sensor (pim400_voltage, "PIM400 -48V_B", &sensor_reading_voltage_b_pim400);

#endif // __APOLLO_SENSOR_INITS_H_
