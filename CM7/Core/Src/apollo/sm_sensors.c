#include "sm_sensors.h"
#include <assert.h>

HAL_StatusTypeDef read_sm_tcn (uint8_t sensor, uint8_t* reading) {

  uint8_t adr=0;

  assert((sensor==TCN_TOP) || (sensor==TCN_MID) || (sensor==TCN_BOT));

  if (sensor==TCN_TOP)
    adr = 0x48;
  else if (sensor==TCN_MID)
    adr = 0x4a;
  else if (sensor==TCN_BOT)
    adr = 0x49;

  // TCN reg 00 = temperature
  // TCN reg 01 = config
  // TCN reg 10 = temperature hysteresis
  // TCN reg 11 = temperature limit set

  uint8_t data [2];
  HAL_StatusTypeDef status = HAL_OK;
  //status |= local_i2c_tx (0x0,  adr); // temperature register 0x0
  status |= local_i2c_rx_n (data, adr, 2);

  // the TCN75 has all sorts of features that really don't matter for us (alerts, etc).
  // lets just keep it simple.. use the default 9bit resolution (0.5C) and read the thing out
  // Register 00
  // | Bit |   15 |  14 |  13 |  12 |  11 |  10 |   9 |   8 |    7 |    6 |    5 |    4 | 3 | 2 | 1 | 0 |
  // | °C  | Sign | 2^6 | 2^5 | 2^4 | 2^3 | 2^2 | 2^1 | 2^0 | 2^-1 | 2^-2 | 2^-3 | 2^-4 | 0 | 0 | 0 | 0 |
  //
  // With 9 bit resolution

  // mask off the sign bit, we don't care about it
  *reading = (0x7F & data[0]);

  return status;

}

void sensor_reading_sm_tcn(uint8_t sensor, sensor_reading_t *sensor_reading) {

  HAL_StatusTypeDef status = HAL_OK;
  uint8_t reading;
  status = read_sm_tcn(sensor, &reading);

  uint8_t temp = reading;

  if (status == HAL_OK) {
    sensor_reading->raw_value = temp;
    sensor_reading->present_state = 0;
  } else {
    sensor_reading->raw_value = 1;
    sensor_reading->present_state = 0;
  }

}

void sensor_reading_sm_tcn_top(sensor_reading_t *sensor_reading) {
  sensor_reading_sm_tcn(TCN_TOP, sensor_reading);
}

void sensor_reading_sm_tcn_mid(sensor_reading_t *sensor_reading) {
  sensor_reading_sm_tcn(TCN_MID, sensor_reading);
}

void sensor_reading_sm_tcn_bot(sensor_reading_t *sensor_reading) {
  sensor_reading_sm_tcn(TCN_BOT, sensor_reading);
}


const linear_sensor_constants_t sm_tcn_temp_consts =
{
  .sensor_type=TEMPERATURE,
  .unit_type=DEGREES_C,
  .lower_nonrecoverable=0,
  .lower_noncritical=0,
  .lower_critical=0,
  .upper_noncritical=38,
  .upper_critical=40,
  .upper_nonrecoverable=50,
  .m=1,
  .b=0,
  .re=1,
  .be=0
};
