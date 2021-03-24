#ifndef __SENSOR_HELPER_H_
#define __SENSOR_HELPER_H_

#include "sdr_definitions.h"

typedef struct  {
  uint8_t sensor_type; // TEMPERATURE, VOLTAGE, CURRENT
  uint8_t unit_type; // DEGREES_C, DEGREES_F, KELVIN, VOLTS, AMPERES, WATTS

  // specified in sensor bits
  uint8_t lower_nonrecoverable;
  uint8_t lower_noncritical;
  uint8_t lower_critical;
  uint8_t upper_noncritical;
  uint8_t upper_critical;
  uint8_t upper_nonrecoverable;
  uint16_t m;
  uint8_t b;
  uint8_t e;
} linear_sensor_constants_t;

void create_linear_sensor (linear_sensor_constants_t params, char* id_string, void (*get_sensor_reading_func)(sensor_reading_t*)) {

  const uint8_t threshold_mask =
    (params.upper_nonrecoverable > 0 ? UPPER_NON_RECOVERABLE  : 0x00) |
    (params.upper_critical       > 0 ? UPPER_CRITICAL         : 0x00) |
    (params.upper_noncritical    > 0 ? UPPER_NON_CRITICAL     : 0x00) |
    (params.lower_nonrecoverable > 0 ? LOWER_NON_RECOVERABLE  : 0x00) |
    (params.lower_critical       > 0 ? LOWER_CRITICAL         : 0x00) |
    (params.lower_noncritical    > 0 ? LOWER_NON_CRITICAL     : 0x00);

  uint8_t threshold_list[6] = {
  params.lower_nonrecoverable,
  params.lower_noncritical,
  params.lower_critical,
  params.upper_noncritical,
  params.upper_critical,
  params.upper_nonrecoverable};

// y = [ M*x + (B * 10^Be ) ] * 10^Re
  create_generic_analog_sensor_1(params.sensor_type, params.unit_type,
                                 params.m,  // m
                                 params.b,  // b
                                 params.e,  // be
                                 -params.e, // re
                                 threshold_mask,
                                 threshold_list,
                                 id_string,
                                 &sensor_reading_voltage_a_pim400 );

}



#endif // __SENSOR_HELPER_H_
