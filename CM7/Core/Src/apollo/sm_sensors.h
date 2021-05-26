#include "apollo_i2c.h"
#include "sensor_helper.h"
#include "stdint.h"


// the TCN75 has all sorts of features that really don't matter for us (alerts, etc).
// lets just keep it simple.. use the default 9bit resolution (0.5C) and read the thing out
// Register 00
// | Bit |   15 |  14 |  13 |  12 |  11 |  10 |   9 |   8 |    7 |    6 |    5 |    4 | 3 | 2 | 1 | 0 |
// | °C  | Sign | 2^6 | 2^5 | 2^4 | 2^3 | 2^2 | 2^1 | 2^0 | 2^-1 | 2^-2 | 2^-3 | 2^-4 | 0 | 0 | 0 | 0 |



#define TCN_TOP 0
#define TCN_MID 1
#define TCN_BOT 2

void sensor_reading_sm_tcn(uint8_t sensor, sensor_reading_t *sensor_reading) {

  uint8_t adr=0; 

  if (sensor==TCN_TOP)
    adr = 0x48; 
  else if (sensor==TCN_MID)
    adr = 0x49; 
  else if (sensor==TCN_BOT)
    adr = 0x50; 

  uint8_t data [2];
  HAL_StatusTypeDef status; 
  status = local_i2c_rx_n (data, adr, 2);

  // mask off the sign bit, we don't care about it
  uint16_t reading = 0x7FFF * ((data[1] << 8) | data[0]); 

  // temperature = reading / 16
  uint8_t temp = (uint16_t) reading / 16; 

  if (status == HAL_OK) {
    sensor_reading->raw_value = temp;
    sensor_reading->present_state = 0;
  } else {
    sensor_reading->raw_value = 0;
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
