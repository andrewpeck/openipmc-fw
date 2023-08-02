#include "cm_sensors.h"

// https://github.com/apollo-lhc/cm_mcu/wiki/MCU-I2C-target-documentation

/* Different sensor types on the Apollo Command Module. */
#define FPGA0 0
#define FPGA1 1
#define FIREFLY 2
#define REGULATOR 3
#define MCU 4

/* I2C address of the CM microcontroller on the CM bus. */
#define CM_MCU_ADR 0x40

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

sensor_reading_status_t sensor_reading_cm_temp(uint8_t sensor, sensor_reading_t *sensor_reading, sensor_thres_values_t *sensor_thresholds) {

  uint8_t tx_data = 0xFF;
  uint8_t rx_data = 0xFF;

  /* Figure out the register address we want to read from the CM MCU. */
  if (sensor == FPGA0) {
    tx_data = 0x12;
  } else if (sensor == FPGA1) {
    tx_data = 0x14;
  } else if (sensor == FIREFLY) {
    tx_data = 0x16;
  } else if (sensor == REGULATOR) {
    tx_data = 0x18;
  } else if (sensor == MCU) {
    tx_data = 0x10;
  }

  /* 
   * Do the I2C transaction. First, write the register address to the CM MCU,
   * and then read the data in that register back.
   */
  const h7i2c_i2c_ret_code_t status_tx = cm1_i2c_tx(&tx_data, CM_MCU_ADR);
  const h7i2c_i2c_ret_code_t status_rx = cm1_i2c_rx(&rx_data, CM_MCU_ADR);

	sensor_reading_status_t sensor_status = SENSOR_READING_OK;

  if ((status_rx == H7I2C_RET_CODE_OK) && (status_tx == H7I2C_RET_CODE_OK)) {

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

  /* Compare the sensor value with the thresholds. If necessary,
    update the present state of this sensor. */
    set_sensor_upper_state(sensor_reading,
        sensor_thresholds->upper_non_critical_threshold,
        sensor_thresholds->upper_critical_threshold,
        sensor_thresholds->upper_non_recoverable_threshold
    );

  return sensor_status;
}


sensor_reading_status_t sensor_reading_cm1_temp(sensor_reading_t *sensor_reading, sensor_thres_values_t *sensor_thresholds) {
  return sensor_reading_cm_temp(FPGA0, sensor_reading, sensor_thresholds);
}

sensor_reading_status_t sensor_reading_cm2_temp(sensor_reading_t *sensor_reading, sensor_thres_values_t *sensor_thresholds) {
  return sensor_reading_cm_temp(FPGA1, sensor_reading, sensor_thresholds);
}

sensor_reading_status_t sensor_reading_cm_firefly_temp(sensor_reading_t *sensor_reading, sensor_thres_values_t *sensor_thresholds) {
  return sensor_reading_cm_temp(FIREFLY, sensor_reading, sensor_thresholds);
}

sensor_reading_status_t sensor_reading_cm_regulator_temp(sensor_reading_t *sensor_reading, sensor_thres_values_t *sensor_thresholds) {
  return sensor_reading_cm_temp(REGULATOR, sensor_reading, sensor_thresholds);
}

sensor_reading_status_t sensor_reading_cm_mcu_temp(sensor_reading_t *sensor_reading, sensor_thres_values_t *sensor_thresholds) {
  return sensor_reading_cm_temp(MCU, sensor_reading, sensor_thresholds);
}

