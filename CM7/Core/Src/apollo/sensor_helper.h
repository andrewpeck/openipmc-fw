#ifndef __SENSOR_HELPER_H_
#define __SENSOR_HELPER_H_

#include <stdint.h>
#include "string.h"
#include "sdr_definitions.h"
#include "sensors_templates.h"

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
  int16_t m;
  int16_t b;
  int8_t be;
  int8_t re;
} linear_sensor_constants_t;

void create_linear_sensor (const linear_sensor_constants_t params,
                           char* id_string,
                           void (*get_sensor_reading_func)(sensor_reading_t*));

void set_sensor_upper_state(sensor_reading_t *sensor_reading,
                      const uint8_t upper_noncritical_raw,
                      const uint8_t upper_critical_raw,
                      const uint8_t upper_nonrecoverable_raw);

void set_sensor_lower_state(sensor_reading_t *sensor_reading,
                      const uint8_t lower_noncritical_raw,
                      const uint8_t lower_critical_raw,
                      const uint8_t lower_nonrecoverable_raw);

#endif // __SENSOR_HELPER_H_
