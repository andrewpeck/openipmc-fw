#include "atlas_temp_sensors.h"
#include <assert.h> 

/* Sensor constans for the ATLAS temperature sensors */
const linear_sensor_constants_t atlas_temp_sensor_consts =
{
  .sensor_type=TEMPERATURE,
  .unit_type=DEGREES_C,
  .lower_nonrecoverable=0,
  .lower_noncritical=0,
  .lower_critical=0,
  .upper_noncritical=50,
  .upper_critical=65,
  .upper_nonrecoverable=80,
  .m=1,
  .b=0,
  .re=0,
  .be=0
};

sensor_reading_status_t sensor_reading_atlas(uint8_t sensor, sensor_reading_t *sensor_reading) {
    HAL_StatusTypeDef status = HAL_OK;

    /* Register address and the slave address to read from. */
    uint8_t reg_addr = 0;
    uint8_t slave_addr = 0;

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

    if (temperature > 0) {
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
                           atlas_temp_sensor_consts.upper_noncritical,
                           atlas_temp_sensor_consts.upper_critical,
                           atlas_temp_sensor_consts.upper_nonrecoverable);

    return sensor_status;

}

sensor_reading_status_t sensor_reading_atlas_board_temp(sensor_reading_t *sensor_reading) {
    return sensor_reading_atlas(BOARD, sensor_reading);
}
sensor_reading_status_t sensor_reading_atlas_firefly_temp(sensor_reading_t *sensor_reading) {
    return sensor_reading_atlas(FIREFLY, sensor_reading);
}
sensor_reading_status_t sensor_reading_atlas_fpga_temp(sensor_reading_t *sensor_reading) {
    return sensor_reading_atlas(FPGA, sensor_reading);
}
sensor_reading_status_t sensor_reading_atlas_pm_temp(sensor_reading_t *sensor_reading) {
    return sensor_reading_atlas(PM, sensor_reading);
}