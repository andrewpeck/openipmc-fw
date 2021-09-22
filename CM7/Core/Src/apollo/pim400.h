#ifndef __PIM400_H_
#define __PIM400_H_

#include "sdr_definitions.h"
#include "sensors_templates.h"
#include "sensor_helper.h"
#include "main.h"

HAL_StatusTypeDef read_temp_pim400 (uint8_t* reading);
HAL_StatusTypeDef read_iout_pim400 (uint8_t* reading);
HAL_StatusTypeDef read_voltage_pim400 (uint8_t* reading, uint8_t supply);

sensor_reading_status_t sensor_reading_temp_pim400(sensor_reading_t *sensor_reading);
sensor_reading_status_t sensor_reading_iout_pim400(sensor_reading_t *sensor_reading);
sensor_reading_status_t sensor_reading_voltage_a_pim400(sensor_reading_t *sensor_reading);
sensor_reading_status_t sensor_reading_voltage_b_pim400(sensor_reading_t *sensor_reading);

#define PIM400_I2C_ADR 0x5E

//------------------------------------------------------------------------------
// Registers
//------------------------------------------------------------------------------

#define PIM400_STATUS_REG 0x1E
#define PIM400_VHOLDUP_REG 0x1F

#define PIM400_IOUT_REG 0x21
#define PIM400_VAF_REG  0x22
#define PIM400_VBF_REG  0x23
#define PIM400_TEMP_REG 0x28

//------------------------------------------------------------------------------
// Temperature
// thresholds in degrees C
//------------------------------------------------------------------------------

// y = [ M*x + (B * 10^Be ) ] * 10^Re
// 
// 1.961 degrees / bit - 50C
//
// y = 1.961*x - 50
// y = (1961 * x - 50 * 1000) / 1000
//
// WARNING: for whatever reason, can't use 1961 and 10^3 here..
// something in the bowels must be 8 bits only

#define PIM400_TEMP_UPPER_NONCRITICAL 50
#define PIM400_TEMP_UPPER_CRITICAL  60
#define PIM400_TEMP_UPPER_NONRECOVERABLE 70
#define PIM400_TEMP_LOWER_NONCRITICAL 0
#define PIM400_TEMP_LOWER_CRITICAL 0
#define PIM400_TEMP_LOWER_NONRECOVERABLE 0
#define PIM400_TEMP_M  196
#define PIM400_TEMP_B (-50)
#define PIM400_TEMP_RE (-2)
#define PIM400_TEMP_BE (2)

static const linear_sensor_constants_t pim400_temp_consts =
{
  .sensor_type=TEMPERATURE,
  .unit_type=DEGREES_C,
  .lower_nonrecoverable = PIM400_TEMP_LOWER_NONRECOVERABLE,
  .lower_critical       = PIM400_TEMP_LOWER_CRITICAL,
  .lower_noncritical    = PIM400_TEMP_LOWER_NONCRITICAL,
  .upper_noncritical    = (PIM400_TEMP_UPPER_CRITICAL + 50) * 100 / 196,
  .upper_critical       = (PIM400_TEMP_UPPER_NONRECOVERABLE + 50) * 100 / 196,
  .upper_nonrecoverable = (PIM400_TEMP_UPPER_NONCRITICAL + 50) * 100 / 196,
  .m                    = PIM400_TEMP_M,
  .b                    = PIM400_TEMP_B,
  .re                   = PIM400_TEMP_RE,
  .be                   = PIM400_TEMP_BE,
};

//------------------------------------------------------------------------------
// Current
//------------------------------------------------------------------------------

// y = [ M*x + (B * 10^Be ) ] * 10^Re
// 
// 0.094A/bit
// 0-23.97A
//
// y = 0.094x
// y = 94x / 1000

#define PIM400_IOUT_UPPER_NONCRITICAL 7
#define PIM400_IOUT_UPPER_CRITICAL 8
#define PIM400_IOUT_UPPER_NONRECOVERABLE 9
#define PIM400_IOUT_LOWER_NONCRITICAL 0
#define PIM400_IOUT_LOWER_CRITICAL 0
#define PIM400_IOUT_LOWER_NONRECOVERABLE 0
#define PIM400_IOUT_M 94
#define PIM400_IOUT_B 0
#define PIM400_IOUT_RE (-3)
#define PIM400_IOUT_BE 0

static const linear_sensor_constants_t pim400_iout_consts =
{
  .sensor_type=CURRENT,
  .unit_type=AMPERES,
  .lower_nonrecoverable = PIM400_IOUT_LOWER_NONRECOVERABLE,
  .lower_critical       = PIM400_IOUT_LOWER_CRITICAL,
  .lower_noncritical    = PIM400_IOUT_LOWER_NONCRITICAL,
  .upper_noncritical    = (1000 * PIM400_IOUT_UPPER_NONCRITICAL) / 94,
  .upper_critical       = (1000 * PIM400_IOUT_UPPER_CRITICAL) / 94,
  .upper_nonrecoverable = (1000 * PIM400_IOUT_UPPER_NONRECOVERABLE) / 94,
  .m                    = PIM400_IOUT_M,
  .b                    = PIM400_IOUT_B,
  .re                   = PIM400_IOUT_RE,
  .be                   = PIM400_IOUT_BE
};

  //------------------------------------------------------------------------------
  // Voltage
  // PIM400 48V input sensors (A and B)
  //------------------------------------------------------------------------------

// y = [ M*x + (B * 10^Be ) ] * 10^Re
// 
// 0.325 V/bit
// 0 -- 82.875V
//
// y = 0.325x
// y = 325x / 1000
#define PIM400_VOLTAGE_UPPER_NONCRITICAL 60
#define PIM400_VOLTAGE_UPPER_CRITICAL 65
#define PIM400_VOLTAGE_UPPER_NONRECOVERABLE 70
#define PIM400_VOLTAGE_LOWER_NONCRITICAL 0
#define PIM400_VOLTAGE_LOWER_CRITICAL 0
#define PIM400_VOLTAGE_LOWER_NONRECOVERABLE 0
#define PIM400_VOLTAGE_M 325
#define PIM400_VOLTAGE_B 0
#define PIM400_VOLTAGE_E (-3)

static const linear_sensor_constants_t pim400_voltage_consts =
{
  .sensor_type          = VOLTAGE,
  .unit_type            = VOLTS,
  .lower_nonrecoverable = (1000 * PIM400_VOLTAGE_UPPER_NONCRITICAL) / 325,
  .lower_critical       = (1000 * PIM400_VOLTAGE_UPPER_CRITICAL) / 325,
  .lower_noncritical    = (1000 * PIM400_VOLTAGE_UPPER_NONRECOVERABLE) / 325,
  .upper_noncritical    = (1000 * PIM400_VOLTAGE_LOWER_NONCRITICAL) / 325,
  .upper_critical       = (1000 * PIM400_VOLTAGE_LOWER_CRITICAL) / 325,
  .upper_nonrecoverable = (1000 * PIM400_VOLTAGE_LOWER_NONRECOVERABLE) / 325,
  .m                    = PIM400_VOLTAGE_M,
  .b                    = PIM400_VOLTAGE_B,
  .re                   = PIM400_VOLTAGE_E,
  .be                   = 0
};


#endif // __PIM400_H_
