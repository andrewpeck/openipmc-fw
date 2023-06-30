#include "atlas_temp_sensors.h"
#include <assert.h> 

/* Sensor constans for the ATLAS temperature sensors */
const linear_sensor_constants_t atlas_temp_sensor_board_consts =
{
  .sensor_type=TEMPERATURE,
  .unit_type=DEGREES_C,
  .lower_nonrecoverable=0,
  .lower_noncritical=0,
  .lower_critical=0,
  .upper_noncritical=50,
  .upper_critical=65,
  .upper_nonrecoverable=70,
  .m=1,
  .b=0,
  .re=0,
  .be=0
};
const linear_sensor_constants_t atlas_temp_sensor_firefly_consts =
{
  .sensor_type=TEMPERATURE,
  .unit_type=DEGREES_C,
  .lower_nonrecoverable=0,
  .lower_noncritical=0,
  .lower_critical=0,
  .upper_noncritical=50,
  .upper_critical=55,
  .upper_nonrecoverable=60,
  .m=1,
  .b=0,
  .re=0,
  .be=0
};
const linear_sensor_constants_t atlas_temp_sensor_fpga_consts =
{
  .sensor_type=TEMPERATURE,
  .unit_type=DEGREES_C,
  .lower_nonrecoverable=0,
  .lower_noncritical=0,
  .lower_critical=0,
  .upper_noncritical=70,
  .upper_critical=80,
  .upper_nonrecoverable=90,
  .m=1,
  .b=0,
  .re=0,
  .be=0
};
const linear_sensor_constants_t atlas_temp_sensor_pm_consts =
{
  .sensor_type=TEMPERATURE,
  .unit_type=DEGREES_C,
  .lower_nonrecoverable=0,
  .lower_noncritical=0,
  .lower_critical=0,
  .upper_noncritical=65,
  .upper_critical=90,
  .upper_nonrecoverable=95,
  .m=1,
  .b=0,
  .re=0,
  .be=0
};

sensor_reading_status_t sensor_reading_atlas(uint8_t sensor, sensor_reading_t *sensor_reading, sensor_thres_values_t *sensor_thresholds) {
    HAL_StatusTypeDef status = HAL_OK;

    /* Register address and the slave address to read from. */
    uint8_t reg_addr = 0;
    uint8_t slave_addr = 0;
    uint8_t rx_data = 0xFF;

    assert((sensor==BOARD) || (sensor==FIREFLY) || (sensor==FPGA) || (sensor==PM));

    if (sensor == BOARD) {
        slave_addr = ZYNQ_I2C_SLAVE2_ADDR;
    }
    else if (sensor == FIREFLY) {
        slave_addr = ZYNQ_I2C_SLAVE3_ADDR;
    }
    else if (sensor == FPGA) {
        slave_addr = ZYNQ_I2C_SLAVE4_ADDR;
    }
    else if (sensor == PM) {
        slave_addr = ZYNQ_I2C_SLAVE5_ADDR;
    }

    /* Read the temperature value from the correct I2C slave device. */
    uint8_t temperature = zynq_rd_reg(reg_addr, slave_addr);

    sensor_reading_status_t sensor_status = SENSOR_READING_OK;

    if (temperature == 0xFF) { // device not powered or present
        sensor_reading->raw_value = 2;
        sensor_status = SENSOR_READING_UNAVAILABLE;
    }
    else if (temperature == 0xFE) { // stale
    //  sensor_reading->raw_value = 0xFE;
        sensor_reading->raw_value = temperature;
        sensor_status = SENSOR_READING_UNAVAILABLE;
    } 
    else if (temperature > 0) {
        sensor_reading->raw_value = temperature;
    }
    /* Read failed. */
    else {
        sensor_reading->raw_value = 0;
        sensor_status = SENSOR_READING_UNAVAILABLE;
    }

    sensor_reading->present_state = 0;

    /* Update the present state of the sensor if an upper threshold is reached. */
    set_sensor_upper_state(sensor_reading,
        sensor_thresholds->upper_non_critical_threshold,
        sensor_thresholds->upper_critical_threshold,
        sensor_thresholds->upper_non_recoverable_threshold);

    return sensor_status;

}

sensor_reading_status_t sensor_reading_atlas_board_temp(sensor_reading_t *sensor_reading, sensor_thres_values_t *sensor_thresholds) {
    return sensor_reading_atlas(BOARD, sensor_reading, sensor_thresholds);
}
sensor_reading_status_t sensor_reading_atlas_firefly_temp(sensor_reading_t *sensor_reading, sensor_thres_values_t *sensor_thresholds) {
    return sensor_reading_atlas(FIREFLY, sensor_reading, sensor_thresholds);
}
sensor_reading_status_t sensor_reading_atlas_fpga_temp(sensor_reading_t *sensor_reading, sensor_thres_values_t *sensor_thresholds) {
    return sensor_reading_atlas(FPGA, sensor_reading, sensor_thresholds);
}
sensor_reading_status_t sensor_reading_atlas_pm_temp(sensor_reading_t *sensor_reading, sensor_thres_values_t *sensor_thresholds) {
    return sensor_reading_atlas(PM, sensor_reading, sensor_thresholds);
}