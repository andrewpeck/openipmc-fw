#ifndef IP_SENSORS_H
#define IP_SENSORS_H

#include "sensor_helper.h"

#include "lwip/netif.h"
#include "lwip/ip4_addr.h"

/*
 * Sensor type and unit definitions for the IP sensors.
 */
#define LAN_SENSOR 0x27
#define BYTE 0x46


sensor_reading_status_t sensor_reading_ipmc_ip_addr(uint8_t byte, sensor_reading_t *sensor_reading);

/*
 * Sensors to read each byte of the IP address.
 * Byte 0 -> Most significant byte
 * Byte 3 -> Least significant byte
 */
sensor_reading_status_t sensor_reading_ipmc_ip_addr_byte0(sensor_reading_t *sensor_reading);
sensor_reading_status_t sensor_reading_ipmc_ip_addr_byte1(sensor_reading_t *sensor_reading);
sensor_reading_status_t sensor_reading_ipmc_ip_addr_byte2(sensor_reading_t *sensor_reading);
sensor_reading_status_t sensor_reading_ipmc_ip_addr_byte3(sensor_reading_t *sensor_reading);

extern const linear_sensor_constants_t ipmc_ip_consts;

#endif