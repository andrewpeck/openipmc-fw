#include "cm_sensors.h"

// https://github.com/apollo-lhc/cm_mcu/wiki/MCU-I2C-target-documentation

#define FPGA0 0
#define FPGA1 1
#define FIREFLY 2
#define REGULATOR 3
#define MCU 4
#define ADR 0x40

const linear_sensor_constants_t cm_fpga_temp_consts = {
    .sensor_type = TEMPERATURE,
    .unit_type = DEGREES_C,
    .lower_nonrecoverable = 0,
    .lower_noncritical = 0,
    .lower_critical = 0,
    .upper_noncritical = 50,
    .upper_critical = 65,
    .upper_nonrecoverable = 80,
    .m = 1,
    .b = 0,
    .re = 0,
    .be = 0};

const linear_sensor_constants_t cm_firefly_temp_consts = {
    .sensor_type = TEMPERATURE,
    .unit_type = DEGREES_C,
    .lower_nonrecoverable = 0,
    .lower_noncritical = 0,
    .lower_critical = 0,
    .upper_noncritical = 44,
    .upper_critical = 50,
    .upper_nonrecoverable = 55,
    .m = 1,
    .b = 0,
    .re = 0,
    .be = 0};

const linear_sensor_constants_t cm_regulator_temp_consts = {
    .sensor_type = TEMPERATURE,
    .unit_type = DEGREES_C,
    .lower_nonrecoverable = 0,
    .lower_noncritical = 0,
    .lower_critical = 0,
    .upper_noncritical = 45,
    .upper_critical = 55,
    .upper_nonrecoverable = 65,
    .m = 1,
    .b = 0,
    .re = 0,
    .be = 0};

const linear_sensor_constants_t cm_mcu_temp_consts = {
    .sensor_type = TEMPERATURE,
    .unit_type = DEGREES_C,
    .lower_nonrecoverable = 0,
    .lower_noncritical = 0,
    .lower_critical = 0,
    .upper_noncritical = 45,
    .upper_critical = 55,
    .upper_nonrecoverable = 65,
    .m = 1,
    .b = 0,
    .re = 0,
    .be = 0};

sensor_reading_status_t sensor_reading_cm_temp(uint8_t sensor, sensor_reading_t *sensor_reading) {

  uint8_t rx_data = 0xFF;
  uint8_t upper_critical=255;
  uint8_t upper_noncritical=255;
  uint8_t upper_nonrecoverable=255;

  if (sensor == FPGA0) {
    rx_data = 0x12;
    upper_noncritical = cm_fpga_temp_consts.upper_noncritical;
    upper_critical = cm_fpga_temp_consts.upper_critical;
    upper_nonrecoverable = cm_fpga_temp_consts.upper_nonrecoverable;
  } else if (sensor == FPGA1) {
    rx_data = 0x14;
    upper_noncritical = cm_fpga_temp_consts.upper_noncritical;
    upper_critical = cm_fpga_temp_consts.upper_critical;
    upper_nonrecoverable = cm_fpga_temp_consts.upper_nonrecoverable;
  } else if (sensor == FIREFLY) {
    rx_data = 0x16;
    upper_noncritical = cm_firefly_temp_consts.upper_noncritical;
    upper_critical = cm_firefly_temp_consts.upper_critical;
    upper_nonrecoverable = cm_firefly_temp_consts.upper_nonrecoverable;
  } else if (sensor == REGULATOR) {
    rx_data = 0x18;
    upper_noncritical = cm_regulator_temp_consts.upper_noncritical;
    upper_critical = cm_regulator_temp_consts.upper_critical;
    upper_nonrecoverable = cm_regulator_temp_consts.upper_nonrecoverable;
  } else if (sensor == MCU) {
    rx_data = 0x10;
    upper_noncritical = cm_mcu_temp_consts.upper_noncritical;
    upper_critical = cm_mcu_temp_consts.upper_critical;
    upper_nonrecoverable = cm_mcu_temp_consts.upper_nonrecoverable;
  }

  HAL_StatusTypeDef status = 0;
  status |= cm1_i2c_tx(&rx_data, ADR);
  status |= cm1_i2c_rx(&rx_data, ADR);

	sensor_reading_status_t sensor_status = SENSOR_READING_OK;

  if (status == HAL_OK) {

    if (rx_data == 0xFF) { // device not powered or present
      sensor_reading->raw_value = 2;
      sensor_status = SENSOR_READING_UNAVAILABLE;
    }
    else if (rx_data == 0xFE) { // stale
      sensor_reading->raw_value = 3;
      sensor_status = SENSOR_READING_UNAVAILABLE;
    }
    else {
      sensor_reading->raw_value = rx_data;
    }
  } else {
    sensor_reading->raw_value = 1;
    sensor_status = SENSOR_READING_UNAVAILABLE;
  }

  sensor_reading->present_state = 0;

  set_sensor_upper_state (sensor_reading,
                          upper_noncritical,
                          upper_critical,
                          upper_nonrecoverable);

  return (sensor_status);
}

sensor_reading_status_t sensor_reading_cm1_temp(sensor_reading_t *sensor_reading) {
  return(sensor_reading_cm_temp(FPGA0, sensor_reading));
}

sensor_reading_status_t sensor_reading_cm2_temp(sensor_reading_t *sensor_reading) {
  return(sensor_reading_cm_temp(FPGA1, sensor_reading));
}

sensor_reading_status_t sensor_reading_cm_firefly_temp(sensor_reading_t *sensor_reading) {
  return(sensor_reading_cm_temp(FIREFLY, sensor_reading));
}

sensor_reading_status_t sensor_reading_cm_regulator_temp(sensor_reading_t *sensor_reading) {
  return(sensor_reading_cm_temp(REGULATOR, sensor_reading));
}

sensor_reading_status_t sensor_reading_cm_mcu_temp(sensor_reading_t *sensor_reading) {
  return(sensor_reading_cm_temp(MCU, sensor_reading));
}

