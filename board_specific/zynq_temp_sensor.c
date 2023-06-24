#include "zynq_temp_sensor.h"
#include "zynq_i2c.h"

// Sensor constants for the Zynq temperature sensor
const linear_sensor_constants_t sm_zynq_temp_consts =
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

sensor_reading_status_t sensor_reading_zynq_temp(sensor_reading_t *sensor_reading, sensor_thres_values_t *sensor_thresholds) {
    /*
     * Function implementing the Zynq temperature sensor.
     */
    uint8_t temp_reading = zynq_get_temperature();

    sensor_reading_status_t sensor_status = SENSOR_READING_OK;

    // If there was an error in the I2C transaction, reading will be returned as 0
    if (temp_reading > 0) {
        sensor_reading->raw_value = temp_reading;
    }
    else {
        sensor_reading->raw_value = 0;
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

