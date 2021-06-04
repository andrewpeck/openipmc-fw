#include <assert.h>
#include "cmsis_os.h"

//OpenIPMC includes
#include "pim400.h"
#include "apollo.h"
#include "../dimm_gpios.h"
#include "../mgm_i2c.h"
#include "power_manager.h"
#include "sdr_definitions.h"

// https://library.industrialsolutions.abb.com/publibrary/checkout/PIM400?TNR=Data%20Sheets|PIM400|PDF
void sensor_reading_temp_pim400 (sensor_reading_t* sensor_reading) {
  uint8_t tx_data, rx_data;
  tx_data = PIM400_TEMP_REG;

  mgm_i2c_transmit(PIM400_I2C_ADR, &tx_data, 1, 100 ); // (adr , data, size, timeout)
  mgm_i2c_receive (PIM400_I2C_ADR, &rx_data, 1, 100 ); // (adr , data, size, timeout)

  // temperature = 1.961 C / bit  - 50C
  sensor_reading->raw_value = rx_data;

  sensor_reading->present_state = 0;

  set_sensor_upper_state (sensor_reading,
                          pim400_temp_upper_noncritical_raw,
                          pim400_temp_upper_critical_raw,
                          pim400_temp_upper_nonrecoverable_raw);

}

void sensor_reading_iout_pim400(sensor_reading_t* sensor_reading) {
  uint8_t tx_data, rx_data;
  tx_data = PIM400_IOUT_REG;

  mgm_i2c_transmit(PIM400_I2C_ADR, &tx_data, 1, 100 ); // (adr , data, size, timeout)
  mgm_i2c_receive (PIM400_I2C_ADR, &rx_data, 1, 100 ); // (adr , data, size, timeout)

  // current = 0.094 A / bit
  sensor_reading->raw_value = rx_data;
  sensor_reading->present_state = 0;

  set_sensor_upper_state (sensor_reading,
                          pim400_iout_upper_noncritical_raw,
                          pim400_iout_upper_critical_raw,
                          pim400_iout_upper_nonrecoverable_raw);

}

void sensor_reading_voltage_pim400(uint8_t supply, sensor_reading_t *sensor_reading) {

  assert(supply == 0 || supply == 1);

  uint8_t tx_data, rx_data;
  if (supply == 0)
    tx_data = PIM400_VAF_REG;
  else
    tx_data = PIM400_VBF_REG;

  mgm_i2c_transmit(PIM400_I2C_ADR, &tx_data, 1, 100); // (adr , data, size, timeout)
  mgm_i2c_receive(PIM400_I2C_ADR, &rx_data, 1, 100); // (adr , data, size, timeout)

  // current = 0.325 V / bit
  sensor_reading->raw_value = rx_data;

  sensor_reading->present_state = 0;

  set_sensor_upper_state (sensor_reading,
                          pim400_voltage_upper_noncritical_raw,
                          pim400_voltage_upper_critical_raw,
                          pim400_voltage_upper_nonrecoverable_raw);

  set_sensor_lower_state (sensor_reading,
                          pim400_voltage_lower_noncritical_raw,
                          pim400_voltage_lower_critical_raw,
                          pim400_voltage_lower_nonrecoverable_raw);


}

void sensor_reading_voltage_a_pim400(sensor_reading_t *sensor_reading) {
  sensor_reading_voltage_pim400(0, sensor_reading);
}

void sensor_reading_voltage_b_pim400(sensor_reading_t *sensor_reading) {
  sensor_reading_voltage_pim400(1, sensor_reading);
}
