#ifndef __PIM400_H_
#define __PIM400_H_

#include "sdr_definitions.h"
#include "sensors_templates.h"
#include "sensor_helper.h"
 
void sensor_reading_temp_pim400(sensor_reading_t *sensor_reading);
void sensor_reading_iout_pim400(sensor_reading_t *sensor_reading);
void sensor_reading_voltage_a_pim400(sensor_reading_t *sensor_reading);
void sensor_reading_voltage_b_pim400(sensor_reading_t *sensor_reading);

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

// 1.961 degrees / bit - 50C
//
// y = 1.961*x - 50
// y = (1961 * x - 50 * 1000) / 1000
//
#define PIM400_TEMP_UPPER_NONCRITICAL 40
#define PIM400_TEMP_UPPER_CRITICAL  60
#define PIM400_TEMP_UPPER_NONRECOVERABLE 90
#define PIM400_TEMP_LOWER_NONCRITICAL 0
#define PIM400_TEMP_LOWER_CRITICAL 0
#define PIM400_TEMP_LOWER_NONRECOVERABLE 0
#define PIM400_TEMP_M  196
#define PIM400_TEMP_B -50I
#define PIM400_TEMP_RE   2
#define PIM400_TEMP_BE   2

static const linear_sensor_constants_t pim400_temp_consts =
{
  .sensor_type=TEMPERATURE,
  .unit_type=DEGREES_C,
  .lower_nonrecoverable = PIM400_TEMP_LOWER_NONRECOVERABLE,
  .lower_critical       = PIM400_TEMP_LOWER_CRITICAL,
  .lower_noncritical    = PIM400_TEMP_LOWER_NONCRITICAL,
  .upper_noncritical    = PIM400_TEMP_UPPER_NONCRITICAL,
  .upper_critical       = PIM400_TEMP_UPPER_CRITICAL,
  .upper_nonrecoverable = PIM400_TEMP_UPPER_NONRECOVERABLE,
  .m                    = PIM400_TEMP_M,
  .b                    = PIM400_TEMP_B,
  .re                   = PIM400_TEMP_RE,
  .be                   = PIM400_TEMP_BE,
};

static const uint8_t pim400_temp_upper_noncritical_raw =
  (PIM400_TEMP_UPPER_NONCRITICAL - PIM400_TEMP_B
          / PIM400_TEMP_M);

static const uint8_t pim400_temp_upper_critical_raw =
  (PIM400_TEMP_UPPER_CRITICAL - PIM400_TEMP_B
          / PIM400_TEMP_M);

static const uint8_t pim400_temp_upper_nonrecoverable_raw =
  (PIM400_TEMP_UPPER_NONRECOVERABLE - PIM400_TEMP_B
          / PIM400_TEMP_M);

//------------------------------------------------------------------------------
// Current
//------------------------------------------------------------------------------

// 0.094A/bit
// 0-23.97A
//
// y = 0.094x
// y = 94x / 1000
#define PIM400_IOUT_UPPER_NONCRITICAL 0
#define PIM400_IOUT_UPPER_CRITICAL 8
#define PIM400_IOUT_UPPER_NONRECOVERABLE 9
#define PIM400_IOUT_LOWER_NONCRITICAL 0
#define PIM400_IOUT_LOWER_CRITICAL 0
#define PIM400_IOUT_LOWER_NONRECOVERABLE 0
#define PIM400_IOUT_M 94
#define PIM400_IOUT_B 0
#define PIM400_IOUT_E 3

static const linear_sensor_constants_t pim400_iout_consts =
{
  .sensor_type=CURRENT,
  .unit_type=AMPERES,
  .lower_nonrecoverable = PIM400_IOUT_LOWER_NONRECOVERABLE,
  .lower_critical       = PIM400_IOUT_LOWER_CRITICAL,
  .lower_noncritical    = PIM400_IOUT_LOWER_NONCRITICAL,
  .upper_noncritical    = PIM400_IOUT_UPPER_NONCRITICAL,
  .upper_critical       = PIM400_IOUT_UPPER_CRITICAL,
  .upper_nonrecoverable = PIM400_IOUT_UPPER_NONRECOVERABLE,
  .m                    = PIM400_IOUT_M,
  .b                    = PIM400_IOUT_B,
  .re                   = PIM400_IOUT_E,
  .be                   = 0
};

static const uint8_t pim400_iout_upper_noncritical_raw =
  (PIM400_IOUT_UPPER_NONCRITICAL - PIM400_IOUT_B
          / PIM400_IOUT_M);

static const uint8_t pim400_iout_upper_critical_raw =
  (PIM400_IOUT_UPPER_CRITICAL - PIM400_IOUT_B
          / PIM400_IOUT_M);

static const uint8_t pim400_iout_upper_nonrecoverable_raw =
  (PIM400_IOUT_UPPER_NONRECOVERABLE - PIM400_IOUT_B
          / PIM400_IOUT_M);

  //------------------------------------------------------------------------------
  // Voltage
  // PIM400 48V input sensors (A and B)
  //------------------------------------------------------------------------------

// 0.325 V/bit
// 0 -- 82.875V
//
// y = 0.325x
// y = 325x / 1000
#define PIM400_VOLTAGE_UPPER_NONCRITICAL 60
#define PIM400_VOLTAGE_UPPER_CRITICAL 65
#define PIM400_VOLTAGE_UPPER_NONRECOVERABLE 70
#define PIM400_VOLTAGE_LOWER_NONCRITICAL 32
#define PIM400_VOLTAGE_LOWER_CRITICAL 36
#define PIM400_VOLTAGE_LOWER_NONRECOVERABLE 40
#define PIM400_VOLTAGE_M 325
#define PIM400_VOLTAGE_B 0
#define PIM400_VOLTAGE_E 3

static const linear_sensor_constants_t pim400_voltage_consts =
{
  .sensor_type          = VOLTAGE,
  .unit_type            = VOLTS,
  .lower_nonrecoverable = PIM400_VOLTAGE_LOWER_NONRECOVERABLE,
  .lower_critical       = PIM400_VOLTAGE_LOWER_CRITICAL,
  .lower_noncritical    = PIM400_VOLTAGE_LOWER_NONCRITICAL,
  .upper_noncritical    = PIM400_VOLTAGE_UPPER_NONCRITICAL,
  .upper_critical       = PIM400_VOLTAGE_UPPER_CRITICAL,
  .upper_nonrecoverable = PIM400_VOLTAGE_UPPER_NONRECOVERABLE,
  .m                    = PIM400_VOLTAGE_M,
  .b                    = PIM400_VOLTAGE_B,
  .re                   = PIM400_VOLTAGE_E,
  .be                   = 0
};

static const uint8_t pim400_voltage_upper_noncritical_raw =
    (PIM400_VOLTAGE_UPPER_NONCRITICAL - PIM400_VOLTAGE_B / PIM400_VOLTAGE_M);

static const uint8_t pim400_voltage_upper_critical_raw =
    (PIM400_VOLTAGE_UPPER_CRITICAL - PIM400_VOLTAGE_B / PIM400_VOLTAGE_M);

static const uint8_t pim400_voltage_upper_nonrecoverable_raw =
    (PIM400_VOLTAGE_UPPER_NONRECOVERABLE - PIM400_VOLTAGE_B / PIM400_VOLTAGE_M);

static const uint8_t pim400_voltage_lower_noncritical_raw =
    (PIM400_VOLTAGE_LOWER_NONCRITICAL - PIM400_VOLTAGE_B / PIM400_VOLTAGE_M);

static const uint8_t pim400_voltage_lower_critical_raw =
    (PIM400_VOLTAGE_LOWER_CRITICAL - PIM400_VOLTAGE_B / PIM400_VOLTAGE_M);

static const uint8_t pim400_voltage_lower_nonrecoverable_raw =
    (PIM400_VOLTAGE_LOWER_NONRECOVERABLE - PIM400_VOLTAGE_B / PIM400_VOLTAGE_M);

#endif // __PIM400_H_
