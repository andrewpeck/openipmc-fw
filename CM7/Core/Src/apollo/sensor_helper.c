#include <stddef.h>
#include <stdint.h>
#include "string.h"
#include "sdr_definitions.h"
#include "sensors_templates.h"
#include "sensor_helper.h"

void create_linear_sensor (const linear_sensor_constants_t params,
                           char* id_string,
                           void (*get_sensor_reading_func)(sensor_reading_t*)) {

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
  create_generic_analog_sensor_1(params.sensor_type,
                                 params.unit_type,
                                 params.m,  // m
                                 params.b,  // b
                                 params.be, // be
                                 params.re, // re
                                 threshold_mask,
                                 threshold_list,
                                 id_string,
                                 get_sensor_reading_func );
}

void set_sensor_upper_state(sensor_reading_t *sensor_reading,
                      const uint8_t upper_noncritical_raw,
                      const uint8_t upper_critical_raw,
                      const uint8_t upper_nonrecoverable_raw) {

  if (sensor_reading->raw_value > upper_noncritical_raw)
    sensor_reading->present_state |= UPPER_NON_CRITICAL;

  if (sensor_reading->raw_value > upper_critical_raw)
    sensor_reading->present_state |= UPPER_CRITICAL;

  if (sensor_reading->raw_value > upper_nonrecoverable_raw)
    sensor_reading->present_state |= UPPER_NON_RECOVERABLE;

}

void set_sensor_lower_state(sensor_reading_t *sensor_reading,
                      const uint8_t lower_noncritical_raw,
                      const uint8_t lower_critical_raw,
                      const uint8_t lower_nonrecoverable_raw) {

  if (sensor_reading->raw_value > lower_noncritical_raw)
    sensor_reading->present_state |= LOWER_NON_CRITICAL;

  if (sensor_reading->raw_value > lower_critical_raw)
    sensor_reading->present_state |= LOWER_CRITICAL;

  if (sensor_reading->raw_value > lower_nonrecoverable_raw)
    sensor_reading->present_state |= LOWER_NON_RECOVERABLE;

}
