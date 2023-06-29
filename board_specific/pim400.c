#include <assert.h>
#include "cmsis_os.h"

//OpenIPMC includes
#include "pim400.h"
#include "apollo.h"
#include "dimm_gpios.h"
#include "mgm_i2c.h"
#include "power_manager.h"
#include "sdr_definitions.h"

// https://library.industrialsolutions.abb.com/publibrary/checkout/PIM400?TNR=Data%20Sheets|PIM400|PDF
// https://www.mouser.com/datasheet/2/167/PIM400_DS-1920379.pdf
//
HAL_StatusTypeDef read_status_pim400 (uint8_t* reading) {
  uint8_t reg_adr;
  reg_adr = PIM400_STATUS_REG;

  HAL_StatusTypeDef status = HAL_OK;
  status |= mgm_i2c_transmit(PIM400_I2C_ADR, &reg_adr, 1, 100 ); // (adr , data, size, timeout)
  status |= mgm_i2c_receive (PIM400_I2C_ADR,  reading, 1, 100 ); // (adr , data, size, timeout)

  return status;
}

HAL_StatusTypeDef read_vholdup_pim400 (uint8_t* reading) {
  uint8_t reg_adr;
  reg_adr = PIM400_VHOLDUP_REG;

  HAL_StatusTypeDef status = HAL_OK;
  status |= mgm_i2c_transmit(PIM400_I2C_ADR, &reg_adr, 1, 100 ); // (adr , data, size, timeout)
  status |= mgm_i2c_receive (PIM400_I2C_ADR,  reading, 1, 100 ); // (adr , data, size, timeout)

  return status;
}

HAL_StatusTypeDef read_temp_pim400 (uint8_t* reading) {
  uint8_t reg_adr;
  reg_adr = PIM400_TEMP_REG;

  HAL_StatusTypeDef status = HAL_OK;
  status |= mgm_i2c_transmit(PIM400_I2C_ADR, &reg_adr, 1, 100 ); // (adr , data, size, timeout)
  status |= mgm_i2c_receive (PIM400_I2C_ADR,  reading, 1, 100 ); // (adr , data, size, timeout)

  return status;
}

sensor_reading_status_t sensor_reading_temp_pim400 (sensor_reading_t* sensor_reading, sensor_thres_values_t *sensor_thresholds) {

  uint8_t rx_data;

  HAL_StatusTypeDef status = read_temp_pim400 (&rx_data);

  // temperature = 1.961 C / bit  - 50C
  if (status == HAL_OK) {
    sensor_reading->raw_value = rx_data;

    sensor_reading->present_state = 0;

    set_sensor_upper_state (sensor_reading,
        sensor_thresholds->upper_non_critical_threshold,
        sensor_thresholds->upper_critical_threshold,
        sensor_thresholds->upper_non_recoverable_threshold
    );

    return(SENSOR_READING_OK);
  } else {
    sensor_reading->raw_value = 0;
    sensor_reading->present_state = 0;
    return(SENSOR_READING_UNAVAILABLE);
  }


}

HAL_StatusTypeDef read_iout_pim400 (uint8_t* reading) {
  uint8_t reg_adr;
  reg_adr = PIM400_IOUT_REG;

  HAL_StatusTypeDef status = HAL_OK;
  status |= mgm_i2c_transmit(PIM400_I2C_ADR, &reg_adr, 1, 100 ); // (adr , data, size, timeout)
  status |= mgm_i2c_receive (PIM400_I2C_ADR,  reading, 1, 100 ); // (adr , data, size, timeout)
  return status;
}

sensor_reading_status_t sensor_reading_iout_pim400(sensor_reading_t* sensor_reading, sensor_thres_values_t *sensor_thresholds) {

  uint8_t rx_data;
  HAL_StatusTypeDef status = read_iout_pim400 (&rx_data);

  // current = 0.094 A / bit
  if (status == HAL_OK) {
    sensor_reading->raw_value = rx_data;
    sensor_reading->present_state = 0;

    set_sensor_upper_state (sensor_reading,
        sensor_thresholds->upper_non_critical_threshold,
        sensor_thresholds->upper_critical_threshold,
        sensor_thresholds->upper_non_recoverable_threshold
    );

    return(SENSOR_READING_OK);
  } else {
    sensor_reading->raw_value = 0;
    sensor_reading->present_state = 0;
    return(SENSOR_READING_UNAVAILABLE);
  }

}

HAL_StatusTypeDef read_voltage_pim400 (uint8_t* reading, uint8_t supply) {

  assert(supply == 0 || supply == 1);

  uint8_t reg_adr;
  if (supply == 0)
    reg_adr = PIM400_VAF_REG;
  else
    reg_adr = PIM400_VBF_REG;

  HAL_StatusTypeDef status = HAL_OK;
  status |= mgm_i2c_transmit(PIM400_I2C_ADR, &reg_adr, 1, 100); // (adr , data, size, timeout)
  status |= mgm_i2c_receive (PIM400_I2C_ADR,  reading, 1, 100); // (adr , data, size, timeout)

  return status;
}

sensor_reading_status_t sensor_reading_voltage_pim400(uint8_t supply, sensor_reading_t *sensor_reading, sensor_thres_values_t *sensor_thresholds) {

  uint8_t rx_data;
  HAL_StatusTypeDef status = read_voltage_pim400 (&rx_data, supply);

  // current = 0.325 V / bit
  if (status == HAL_OK) {
    sensor_reading->raw_value = rx_data;

    sensor_reading->present_state = 0;

    set_sensor_upper_state (sensor_reading,
        sensor_thresholds->upper_non_critical_threshold,
        sensor_thresholds->upper_critical_threshold,
        sensor_thresholds->upper_non_recoverable_threshold
    );

    return(SENSOR_READING_OK);
  } else {
    sensor_reading->raw_value = 0;
    sensor_reading->present_state = 0;
    return(SENSOR_READING_UNAVAILABLE);
  }
}

sensor_reading_status_t sensor_reading_voltage_a_pim400(sensor_reading_t *sensor_reading, sensor_thres_values_t *sensor_thresholds) {
  return(sensor_reading_voltage_pim400(0, sensor_reading, sensor_thresholds));
}

sensor_reading_status_t sensor_reading_voltage_b_pim400(sensor_reading_t *sensor_reading, sensor_thres_values_t *sensor_thresholds) {
  return(sensor_reading_voltage_pim400(1, sensor_reading, sensor_thresholds));
}
