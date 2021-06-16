#include "cm_sensors.h"

#define FPGA0 0
#define FPGA1 1
#define FIREFLY 2
#define REGULATOR 3

void sensor_reading_cm_temp(uint8_t sensor, sensor_reading_t *sensor_reading) {

  uint8_t adr = 0x0;

  if (sensor==FPGA0)
    adr = 0x61;
  else if (sensor==FPGA1)
    adr = 0x63;
  else if (sensor==FIREFLY)
    adr = 0x62;
  else if (sensor==REGULATOR)
    adr = 0x64;

  uint8_t rx_data;
  HAL_StatusTypeDef status = cm1_i2c_rx(&rx_data, adr);

  if (status == HAL_OK) {
    sensor_reading->raw_value = rx_data;
  } else {
    sensor_reading->raw_value = 0;
  }

  sensor_reading->present_state = 0;

  // if (sensor_reading->raw_value > sensor_reading->upper_noncritical)
  //   sensor_reading->present_state |= UPPER_NON_CRITICAL;
  // if (sensor_reading->raw_value > sensor_reading->upper_critical)
  //   sensor_reading->present_state |= UPPER_CRITICAL;
  // if (sensor_reading->raw_value > sensor_reading->upper_nonrecoverable)
  //   sensor_reading->present_state |= UPPER_NON_RECOVERABLE;
}


void sensor_reading_cm1_temp(sensor_reading_t *sensor_reading) {
  sensor_reading_cm_temp(FPGA0, sensor_reading);
}

void sensor_reading_cm2_temp(sensor_reading_t *sensor_reading) {
  sensor_reading_cm_temp(FPGA1, sensor_reading);
}

void sensor_reading_cm_firefly_temp(sensor_reading_t *sensor_reading) {
  sensor_reading_cm_temp(FIREFLY, sensor_reading);
}

void sensor_reading_cm_regulator_temp(sensor_reading_t *sensor_reading) {
  sensor_reading_cm_temp(REGULATOR, sensor_reading);
}

const linear_sensor_constants_t cm_fpga_temp_consts =
{
  .sensor_type          = TEMPERATURE,
  .unit_type            = DEGREES_C,
  .lower_nonrecoverable = 0,
  .lower_noncritical    = 0,
  .lower_critical       = 0,
  .upper_noncritical    = 50,
  .upper_critical       = 65,
  .upper_nonrecoverable = 80,
  .m                    = 1,
  .b                    = 0,
  .re                   = 0,
  .be                   = 0
};

const linear_sensor_constants_t cm_firefly_temp_consts =
{
  .sensor_type          = TEMPERATURE,
  .unit_type            = DEGREES_C,
  .lower_nonrecoverable = 0,
  .lower_noncritical    = 0,
  .lower_critical       = 0,
  .upper_noncritical    = 44,
  .upper_critical       = 50,
  .upper_nonrecoverable = 55,
  .m                    = 1,
  .b                    = 0,
  .re                   = 0,
  .be                   = 0
};

const linear_sensor_constants_t cm_regulator_temp_consts =
{
  .sensor_type          = TEMPERATURE,
  .unit_type            = DEGREES_C,
  .lower_nonrecoverable = 0,
  .lower_noncritical    = 0,
  .lower_critical       = 0,
  .upper_noncritical    = 45,
  .upper_critical       = 55,
  .upper_nonrecoverable = 65,
  .m                    = 1,
  .b                    = 0,
  .re                   = 0,
  .be                   = 0
};
