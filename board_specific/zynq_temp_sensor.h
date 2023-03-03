#ifndef ZYNQ_TEMP_SENSOR_H
#define ZYNQ_TEMP_SENSOR_H

#include "zynq_i2c.h"
#include "sensor_helper.h"

sensor_reading_status_t sensor_reading_zynq_temp(sensor_reading_t *sensor_reading);
extern const linear_sensor_constants_t sm_zynq_temp_consts;

#endif /* ZYNQ_TEMP_SENSOR_H */