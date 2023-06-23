#include <stddef.h>
#include <stdint.h>
#include "string.h"
#include "sdr_definitions.h"
#include "sensors_templates.h"
#include "sensor_helper.h"

void create_linear_sensor (const linear_sensor_constants_t params,
                           char* id_string,
                           sensor_reading_status_t (*get_sensor_reading_func)(sensor_reading_t*, sensor_thres_values_t*)) {

  /* Mask representing which thresholds are settable and readable. */
  const uint8_t threshold_mask_set =
    (params.upper_nonrecoverable > 0 ? UPPER_NON_RECOVERABLE  : 0x00) |
    (params.upper_critical       > 0 ? UPPER_CRITICAL         : 0x00) |
    (params.upper_noncritical    > 0 ? UPPER_NON_CRITICAL     : 0x00) |
    (params.lower_nonrecoverable > 0 ? LOWER_NON_RECOVERABLE  : 0x00) |
    (params.lower_critical       > 0 ? LOWER_CRITICAL         : 0x00) |
    (params.lower_noncritical    > 0 ? LOWER_NON_CRITICAL     : 0x00);
  
  /* Mask representing which thresholds are readable. If the above mask already is 0x1
  for the given threshold type, then this is redundant. */
  const uint8_t threshold_mask_read =
    (params.upper_nonrecoverable > 0 ? UPPER_NON_RECOVERABLE  : 0x00) |
    (params.upper_critical       > 0 ? UPPER_CRITICAL         : 0x00) |
    (params.upper_noncritical    > 0 ? UPPER_NON_CRITICAL     : 0x00) |
    (params.lower_nonrecoverable > 0 ? LOWER_NON_RECOVERABLE  : 0x00) |
    (params.lower_critical       > 0 ? LOWER_CRITICAL         : 0x00) |
    (params.lower_noncritical    > 0 ? LOWER_NON_CRITICAL     : 0x00);

  uint8_t threshold_list[6] = {
      params.lower_nonrecoverable,  // 0 = lower non recoverable
      params.lower_critical,        // 1 = lower critical
      params.lower_noncritical,     // 2 = lower non-critcal
      params.upper_noncritical,     // 3 = upper non-critical
      params.upper_critical,        // 4 = upper critical
      params.upper_nonrecoverable}; // 5 = upper non recoverable

  /* 
   * Struct of parameters for this sensor. This will be passed to 
   * create_generic_analog_sensor_1() function call to create the entry
   * for this sensor.
   * 
   * (m, b) and (b_exp, r_exp) below refer to the following transformation on the input:
   * y = [ m*x + (b * 10^b_exp ) ] * 10^r_exp
   * 
   */
  analog_sensor_1_init_t sensor_params = {
    .sensor_type=params.sensor_type,
    .base_unit_type=params.unit_type,
    .m=params.m,
    .b=params.b,
    .b_exp=params.be,
    .r_exp=params.re,
    .threshold_mask_set=threshold_mask_set,   /* > 0 means that this threshold is settable AND readable via ipmitool. */
    .threshold_mask_read=threshold_mask_read, /* If set == 0 (above), read > 0 means that this threshold is readable. */
    .threshold_list=threshold_list,
    .id_string=id_string,
    .get_sensor_reading_func=get_sensor_reading_func,
    .sensor_action_req=NULL /* Currently, we don't support sensor action callbacks specific to Apollo usage. */
  };

  create_generic_analog_sensor_1(&sensor_params);

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
