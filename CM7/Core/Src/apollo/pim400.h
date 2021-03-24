#ifndef __PIM400_H_
#define __PIM400_H_

#include "sdr_definitions.h"
#include "sensors_templates.h"

void sensor_reading_temp_pim400(sensor_reading_t* sensor_reading);
void sensor_reading_iout_pim400(sensor_reading_t* sensor_reading);
void sensor_reading_voltage_a_pim400(sensor_reading_t* sensor_reading);
void sensor_reading_voltage_b_pim400(sensor_reading_t* sensor_reading);

#define PIM400_I2C_ADR 0x5E

//------------------------------------------------------------------------------
// Status register
//------------------------------------------------------------------------------

#define PIM400_STATUS_REG 0x1E

//------------------------------------------------------------------------------
// VHoldup measurement
//------------------------------------------------------------------------------
#define PIM400_VHOLDUP_REG 0x1F

//------------------------------------------------------------------------------
// Current measurement
//------------------------------------------------------------------------------
#define PIM400_IOUT_REG 0x21

// thresholds in Amps
#define PIM400_IOUT_NONCRITICAL 8.0
#define PIM400_IOUT_CRITICAL 9.0
#define PIM400_IOUT_CONV 0.094

#define PIM400_IOUT_M 0.094
#define PIM400_IOUT_B 0
#define PIM400_IOUT_BE 0
#define PIM400_IOUT_RE 1000

//------------------------------------------------------------------------------
// Voltage measurement
//------------------------------------------------------------------------------
#define PIM400_VAF_REG 0x22
#define PIM400_VBF_REG 0x23

// thresholds in volts
#define PIM400_V_CONV 0.325
#define PIM400_VLO_NONCRITICAL 40.0
#define PIM400_VLO_CRITICAL 36.0
#define PIM400_VHI_NONCRITICAL 64.0
#define PIM400_VHI_CRITICAL 70.0

//------------------------------------------------------------------------------
// Temperature measurement
//------------------------------------------------------------------------------
// thresholds in degrees C
#define PIM400_TEMP_REG 0x28
#define PIM400_TEMP_NONCRITICAL 70
#define PIM400_TEMP_CRITICAL 80

#define PIM400_TEMP_M 1.961
#define PIM400_TEMP_B -50
#define PIM400_TEMP_BE 3
#define PIM400_TEMP_RE 3



#endif // __PIM400_H_
