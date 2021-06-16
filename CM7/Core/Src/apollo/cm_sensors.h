#ifndef CM_SENSORS_H
#define CM_SENSORS_H

#include "sensor_helper.h"
#include "apollo_i2c.h"

#define FPGA0 0
#define FPGA1 1
#define FIREFLY 2
#define REGULATOR 3

void sensor_reading_cm_temp(uint8_t sensor, sensor_reading_t *sensor_reading);
void sensor_reading_cm1_temp(sensor_reading_t *sensor_reading);
void sensor_reading_cm2_temp(sensor_reading_t *sensor_reading);
void sensor_reading_cm_firefly_temp(sensor_reading_t *sensor_reading);
void sensor_reading_cm_regulator_temp(sensor_reading_t *sensor_reading);

extern const linear_sensor_constants_t cm_fpga_temp_consts;
extern const linear_sensor_constants_t cm_firefly_temp_consts;
extern const linear_sensor_constants_t cm_regulator_temp_consts;

#endif /* CM_SENSORS_H */
