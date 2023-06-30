#ifndef ATLAS_TEMP_SENSORS_H
#define ATLAS_TEMP_SENSORS_H

#include "zynq_i2c.h"
#include "sensor_helper.h"

/* Define different types of sensors we're going to read from. */
#define BOARD 0
#define FIREFLY 1
#define FPGA 2
#define PM 3


sensor_reading_status_t sensor_reading_atlas(uint8_t sensor, sensor_reading_t *sensor_reading, sensor_thres_values_t *sensor_thresholds);

sensor_reading_status_t sensor_reading_atlas_board_temp(sensor_reading_t *sensor_reading, sensor_thres_values_t *sensor_thresholds);
sensor_reading_status_t sensor_reading_atlas_firefly_temp(sensor_reading_t *sensor_reading, sensor_thres_values_t *sensor_thresholds);
sensor_reading_status_t sensor_reading_atlas_fpga_temp(sensor_reading_t *sensor_reading, sensor_thres_values_t *sensor_thresholds);
sensor_reading_status_t sensor_reading_atlas_pm_temp(sensor_reading_t *sensor_reading, sensor_thres_values_t *sensor_thresholds);

extern const linear_sensor_constants_t atlas_temp_sensor_board_consts;
extern const linear_sensor_constants_t atlas_temp_sensor_firefly_consts;
extern const linear_sensor_constants_t atlas_temp_sensor_fpga_consts;
extern const linear_sensor_constants_t atlas_temp_sensor_pm_consts;

#endif /* ATLAS_TEMP_SENSORS_H */