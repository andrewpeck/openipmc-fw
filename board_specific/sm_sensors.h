#ifndef SM_SENSORS_H
#define SM_SENSORS_H

#include "apollo_i2c.h"
#include "sensor_helper.h"
#include "h7i2c_bare.h"
#include "stdint.h"

#define TCN_TOP 0
#define TCN_MID 1
#define TCN_BOT 2

h7i2c_i2c_ret_code_t read_sm_tcn (uint8_t sensor, uint8_t* reading);
h7i2c_i2c_ret_code_t read_sm_tcn_raw (uint8_t sensor, uint8_t* data);
sensor_reading_status_t sensor_reading_sm_tcn(uint8_t sensor, sensor_reading_t *sensor_reading);
sensor_reading_status_t sensor_reading_sm_tcn_top(sensor_reading_t *sensor_reading);
sensor_reading_status_t sensor_reading_sm_tcn_mid(sensor_reading_t *sensor_reading);
sensor_reading_status_t sensor_reading_sm_tcn_bot(sensor_reading_t *sensor_reading);
extern const linear_sensor_constants_t sm_tcn_temp_consts;

#endif /* SM_SENSORS_H */
